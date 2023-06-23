//
// Created by Zero on 2023/6/12.
//

#include "pipeline.h"

namespace vision {

BakerPipeline::BakerPipeline(const PipelineDesc &desc)
    : Pipeline(desc),
      _uv_unwrapper(Global::node_mgr().load<UVUnwrapper>(desc.unwrapper_desc)),
      _rasterizer(Global::node_mgr().load<Rasterizer>(desc.rasterizer_desc)) {
    create_cache_directory_if_necessary();
}

void BakerPipeline::compile_transform_shader() noexcept {
    Kernel kernel = [&](BufferVar<float4> positions,
                        BufferVar<float4> normals, Float4x4 o2w) {
        Float4 position = positions.read(dispatch_id());
        Float4 normal = normals.read(dispatch_id());
        $if(position.w > 0.f) {
            Float3 world_pos = transform_point(o2w, position.xyz());
            Float3 world_norm = transform_normal(o2w, normal.xyz());
            positions.write(dispatch_id(), make_float4(world_pos, position.w));
            normals.write(dispatch_id(), make_float4(world_norm, normal.w));
        };
    };
    _transform_shader = device().compile(kernel, "transform shader");
}

void BakerPipeline::init_scene(const vision::SceneDesc &scene_desc) {
    _scene.init(scene_desc);
    init_postprocessor(scene_desc);
}

void BakerPipeline::init_postprocessor(const vision::SceneDesc &scene_desc) {
    _postprocessor.set_denoiser(_scene.load<Denoiser>(scene_desc.denoiser_desc));
    _postprocessor.set_tone_mapper(_scene.camera()->radiance_film()->tone_mapper());
}

void BakerPipeline::prepare() noexcept {
    auto pixel_num = resolution().x * resolution().y;
    _final_picture.reset_all(device(), pixel_num);
    _scene.prepare();
    image_pool().prepare();
    preprocess();
    prepare_geometry();
    compile_shaders();
    prepare_resource_array();
    bake_all();
    upload_lightmap();
    prepare_resource_array();
}

void BakerPipeline::preprocess() noexcept {
    // fill baked shape list
    for_each_need_bake([&](Shape *item) {
        _baked_shapes.emplace_back(item);
    });

    // uv spread
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        UnwrapperResult spread_result;
        if (baked_shape.has_uv_cache()) {
            spread_result = baked_shape.load_uv_config_from_cache();
        } else {
            spread_result = _uv_unwrapper->apply(baked_shape.shape());
            baked_shape.save_to_cache(spread_result);
        }
        baked_shape.setup_vertices(ocarina::move(spread_result));
    });

    // rasterize
    _rasterizer->compile_shader();
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        baked_shape.prepare_for_rasterize();
    });
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        if (baked_shape.has_rasterization_cache()) {
            baked_shape.load_rasterization_from_cache();
        } else {
            _rasterizer->apply(baked_shape);
        }
    });
    stream() << synchronize() << commit();

    // save rasterize cache
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        baked_shape.normalize_lightmap_uv();
        if (!baked_shape.has_rasterization_cache()) {
            baked_shape.save_rasterization_to_cache();
        }
    });

    // transform to world space
    compile_transform_shader();
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        stream() << _transform_shader(baked_shape.positions(),
                                      baked_shape.normals(),
                                      baked_shape.shape()->o2w())
                        .dispatch(baked_shape.resolution());
    });
    stream() << synchronize() << commit();
}

RayState BakerPipeline::generate_ray(const Float4 &position, const Float4 &normal, Float *scatter_pdf) const noexcept {
    Sampler *sampler = scene().sampler();
    Float3 wi = square_to_cosine_hemisphere(sampler->next_2d());
    *scatter_pdf = cosine_hemisphere_PDF(wi.z);
    Frame frame(normal.xyz());
    OCRay ray = vision::spawn_ray(position.xyz(), normal.xyz(), frame.to_world(wi));
    return {.ray = ray, .ior = 1.f, .medium = InvalidUI32};
}

void BakerPipeline::compile_shaders() noexcept {
    compile_baker();
        _scene.integrator()->compile_shader();
    compile_displayer();
}

void BakerPipeline::compile_baker() noexcept {
    Sampler *sampler = scene().sampler();
    Kernel bake_kernel = [&](Uint frame_index, BufferVar<float4> positions,
                             BufferVar<float4> normals, BufferVar<float4> lightmap) {
        Uint pixel_index = dispatch_id();
        Float4 position = positions.read(pixel_index);
        Float4 normal = normals.read(pixel_index);

        $if(position.w > 0.5f) {
            sampler->start_pixel_sample(dispatch_idx().xy(), frame_index, 0);
            Float scatter_pdf;
            RayState rs = generate_ray(position, normal, &scatter_pdf);
            Float3 L = integrator()->Li(rs, scatter_pdf);
            Float3 accum_prev = lightmap.read(pixel_index).xyz();
            Float a = 1.f / (frame_index + 1);
            L = lerp(make_float3(a), accum_prev, L);
            lightmap.write(pixel_index, make_float4(L, 1.f));
        };
    };
    _bake_shader = device().compile(bake_kernel, "bake kernel");
}

Float3 BakerPipeline::Li(vision::RayState &rs) const noexcept {
    Float3 L = make_float3(0.f);
    Var hit = geometry().trace_closest(rs.ray);
    Interaction it = geometry().compute_surface_interaction(hit, rs.ray);
    comment("resource_array start");
    L = resource_array().tex(0).sample(3, it.lightmap_uv).as_vec3();
    comment("resource_array end");
    return make_float3(L);
}

void BakerPipeline::compile_displayer() noexcept {
    Camera *camera = scene().camera();
    Sampler *sampler = scene().sampler();
    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        sampler->start_pixel_sample(pixel, frame_index, 0);
        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        camera->load_data();
        Float scatter_pdf = 1e16f;
        RayState rs = camera->generate_ray(ss);

        Float3 L = make_float3(0.f);

        Var hit = geometry().trace_closest(rs.ray);
        $if(hit->is_miss()) {
            camera->radiance_film()->add_sample(pixel, L, frame_index);
            $return();
        };

        Interaction it = geometry().compute_surface_interaction(hit, rs.ray);

        $if(it.has_lightmap()) {
            L = resource_array().tex(1 + it.lightmap_id).sample(3, it.lightmap_uv).as_vec3();
        };
        camera->radiance_film()->add_sample(pixel, L, frame_index);
    };
    _display_shader = device().compile(kernel, "display");
}

void BakerPipeline::bake(vision::BakedShape &baked_shape) noexcept {
    Context::create_directory_if_necessary(baked_shape.instance_cache_directory());
    Sampler *sampler = scene().sampler();
    OC_INFO_FORMAT("start bake {}", baked_shape.shape()->name());
    for (int i = 0; i < sampler->sample_per_pixel(); ++i) {
        stream() << _bake_shader(i, baked_shape.positions(),
                                 baked_shape.normals(),
                                 baked_shape.lightmap())
                        .dispatch(baked_shape.resolution());
    }
}

void BakerPipeline::bake_all() noexcept {
    // bake
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        baked_shape.prepare_for_bake();
        bake(baked_shape);
    });
    stream() << synchronize() << commit();
    for (int i = 0; i < _baked_shapes.size(); ++i) {
        _baked_shapes[i].save_lightmap_to_cache();
        _baked_shapes[i].shape()->handle().lightmap_id = i;
    }
}

void BakerPipeline::upload_lightmap() noexcept {
    _lightmap_base_index = pipeline()->resource_array().texture_num();
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        baked_shape.allocate_lightmap_texture();
        vector<float4> data;
        auto cmd = baked_shape.lightmap_tex().copy_from_buffer_sync(baked_shape.lightmap().handle(), 0);
        stream() << cmd << synchronize() << commit();
        register_texture(baked_shape.lightmap_tex());
    });

    stream() << synchronize() << commit();
}

void BakerPipeline::render(double dt) noexcept {
    Clock clk;
//        integrator()->render();
    stream() << _display_shader(frame_index()).dispatch(resolution());
    stream() << synchronize();
    stream() << commit();
    double ms = clk.elapse_ms();
    Printer::instance().retrieve_immediately();
    _total_time += ms;
    ++_frame_index;
    printf("time consuming (current frame: %f, average: %f) frame index: %u\r", ms, _total_time / _frame_index, _frame_index);
}

void BakerPipeline::display(double dt) noexcept {
    render(dt);
}

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BakerPipeline)