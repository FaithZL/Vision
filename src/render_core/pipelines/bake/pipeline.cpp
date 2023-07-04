//
// Created by Zero on 2023/6/12.
//

#include "pipeline.h"

namespace vision {

BakePipeline::BakePipeline(const PipelineDesc &desc)
    : Pipeline(desc),
      _uv_unwrapper(Global::node_mgr().load<UVUnwrapper>(desc.unwrapper_desc)),
      _baker(Global::node_mgr().load<Rasterizer>(desc.rasterizer_desc)) {
    create_cache_directory_if_necessary();
}

void BakePipeline::init_scene(const vision::SceneDesc &scene_desc) {
    _scene.init(scene_desc);
    init_postprocessor(scene_desc);
}

void BakePipeline::init_postprocessor(const vision::SceneDesc &scene_desc) {
    _postprocessor.set_denoiser(_scene.load<Denoiser>(scene_desc.denoiser_desc));
    _postprocessor.set_tone_mapper(_scene.camera()->radiance_film()->tone_mapper());
}

void BakePipeline::prepare() noexcept {
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
    update_geometry();
    prepare_resource_array();
}

void BakePipeline::preprocess() noexcept {
    // fill baked shape list
    for_each_need_bake([&](Shape *item) {
        _baked_shapes.emplace_back(item);
    });

    // uv unwrap
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        UnwrapperResult unwrap_result;
        if (baked_shape.has_uv_cache()) {
            unwrap_result = baked_shape.load_uv_config_from_cache();
        } else {
            unwrap_result = _uv_unwrapper->apply(baked_shape.shape());
            baked_shape.save_to_cache(unwrap_result);
        }
        baked_shape.setup_vertices(ocarina::move(unwrap_result));
    });
}

void BakePipeline::compile_shaders() noexcept {
    _scene.integrator()->compile_shader();
    compile_displayer();
}

void BakePipeline::compile_displayer() noexcept {
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

void BakePipeline::bake_all() noexcept {
    _baker.allocate();
    _baker.compile();
//    std::sort(_baked_shapes.begin(), _baked_shapes.end(),
//              [&](const BakedShape &a, const BakedShape &b) {
//                  return a.perimeter() > b.perimeter();
//              });

    for (auto iter = _baked_shapes.begin(); iter != _baked_shapes.end();) {
        uint pixel_num = 0;
        auto it = iter;
        for (; it != _baked_shapes.end(); ++it) {
            if (pixel_num + it->pixel_num() <= _baker.buffer_size()) {
                pixel_num += it->pixel_num();
            } else {
                break;
            }
        }
        _baker.baking(ocarina::span(iter, it));
        iter = it;
    }
}

void BakePipeline::upload_lightmap() noexcept {
    _lightmap_base_index = pipeline()->resource_array().texture_num();
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        baked_shape.allocate_lightmap_texture();
        vector<float4> data;
        auto cmd = baked_shape.lightmap_tex().copy_from_buffer(baked_shape.lightmap().handle(), 0);
        stream() << cmd << synchronize() << commit();
        register_texture(baked_shape.lightmap_tex());
    });

    stream() << synchronize() << commit();
}

void BakePipeline::render(double dt) noexcept {
    Clock clk;
    stream() << _display_shader(frame_index()).dispatch(resolution());
    stream() << synchronize();
    stream() << commit();
    double ms = clk.elapse_ms();
    Printer::instance().retrieve_immediately();
    _total_time += ms;
    ++_frame_index;
    printf("time consuming (current frame: %f, average: %f) frame index: %u   \r", ms, _total_time / _frame_index, _frame_index);
}

void BakePipeline::display(double dt) noexcept {
    render(dt);
}

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BakePipeline)