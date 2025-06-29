//
// Created by Zero on 2023/6/12.
//

#include "pipeline.h"
#include "core/thread_pool.h"

namespace vision {

BakePipeline::BakePipeline(const PipelineDesc &desc)
    : Pipeline(desc),
      _desc(desc) {
    create_cache_directory_if_necessary();
}

void BakePipeline::init_scene(const vision::SceneDesc &scene_desc) {
    scene_.init(scene_desc);
    init_postprocessor(scene_desc.denoiser_desc);
    postprocessor_.set_tone_mapper(scene_.sensor()->rad_collector()->tone_mapper());
}

void BakePipeline::init_postprocessor(const DenoiserDesc &desc) {
    postprocessor_.set_denoiser(Node::create_shared<Denoiser>(desc));
}

void BakePipeline::prepare() noexcept {
    Pipeline::prepare();
    auto pixel_num = resolution().x * resolution().y;
    final_picture_.reset_all(device(), pixel_num);
    scene_.prepare();
    image_pool().prepare();
    preprocess();
    prepare_geometry();
    compile();
    upload_bindless_array();
    lightmap_base_index_ = pipeline()->bindless_array().texture_num();
    bake_all();
    update_geometry();
    upload_bindless_array();
}

void BakePipeline::preprocess() noexcept {
    // fill baked shape list
    for_each_need_bake([&](SP<ShapeInstance> item) {
        baked_shapes_.emplace_back(item.get());
    });
    SP<UVUnwrapper> uv_unwrapper = Node::create_shared<UVUnwrapper>(_desc.unwrapper_desc);

    // uv unwrap
    VS_BAKER_STATS(baker_stats_, uv_unwrap)
    std::for_each(baked_shapes_.begin(), baked_shapes_.end(), [&](BakedShape &baked_shape) {
        Mesh *mesh = baked_shape.shape()->mesh().get();
        if (mesh->has_lightmap_uv()) {
            return ;
        }
        UnwrapperResult unwrap_result;
        if (baked_shape.has_uv_cache()) {
            unwrap_result = baked_shape.load_uv_config_from_cache();
        } else {
            unwrap_result = uv_unwrapper->apply(baked_shape.shape()->mesh().get());
            baked_shape.save_to_cache(unwrap_result);
        }
        mesh->setup_lightmap_uv(unwrap_result);
    });
}

void BakePipeline::compile() noexcept {
    Pipeline::compile();
    compile_displayer();
}

void BakePipeline::compile_displayer() noexcept {
    TSensor &camera = scene().sensor();
    TSampler &sampler = scene().sampler();
    Kernel kernel = [&](Uint frame_index, Uint lightmap_base) {
        Uint2 pixel = dispatch_idx().xy();
        camera->load_data();
        sampler->start(pixel, frame_index, 0);
        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        RayState rs = camera->generate_ray(ss);

        Float3 L = make_float3(0.f);

        Var hit = geometry().trace_closest(rs.ray);
        $if(hit->is_miss()) {
            camera->rad_collector()->add_sample(pixel, L, frame_index);
            $return();
        };

        Interaction it = geometry().compute_surface_interaction(hit, rs.ray);

        $if(it.has_lightmap()) {
            Float4 t = bindless_array().tex_var(lightmap_base + it.lightmap_id).sample(4, it.lightmap_uv).as_vec4();
            L = t.xyz() / t.w;
            $if(has_invalid(L)) {
                L = make_float3(0.f);
            };
        };
        camera->rad_collector()->add_sample(pixel, L, frame_index);
    };
    display_shader_ = device().compile(kernel, "display");
}

void BakePipeline::bake_all() noexcept {
    Baker baker{baker_stats_, Rasterizer (_desc.rasterizer_desc)};
    baker.allocate();
    baker.compile();
    baker_stats_.set_model_num(baked_shapes_.size());

    async([&] {
        while (baker_stats_.is_valid()) {
            baker_stats_.report_progress();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    baker_stats_.set_spp(scene().sampler()->sample_per_pixel());
    size_t bake_pixel_num = 0;
    std::sort(baked_shapes_.begin(), baked_shapes_.end(),
              [&](const BakedShape &a, const BakedShape &b) {
                  return a.perimeter() > b.perimeter();
              });
    std::for_each(baked_shapes_.begin(), baked_shapes_.end(), [&](const BakedShape &bs) {
        bake_pixel_num += bs.pixel_num();
    });
    baker_stats_.set_pixel_num(bake_pixel_num);

    for (int i = 0; i < baked_shapes_.size(); ++i) {
        BakedShape &bs = baked_shapes_[i];
        bs.shape()->set_lightmap_id(i);
    }

    for (auto iter = baked_shapes_.begin(); iter != baked_shapes_.end();) {
        uint pixel_num = 0;
        auto it = iter;
        for (; it != baked_shapes_.end(); ++it) {
            if (pixel_num + it->pixel_num() <= baker.buffer_size()) {
                pixel_num += it->pixel_num();
            } else {
                break;
            }
        }
        baker.baking(ocarina::span(iter, it));
        iter = it;
    }

    std::for_each(baked_shapes_.begin(), baked_shapes_.end(), [&](BakedShape &bs) {
        bs.normalize_lightmap_uv();
    });

    stream() << baker.deallocate()
             << synchronize() << commit();
    OC_INFO(baker_stats_.get_all_stats());
    baker_stats_.clear();
}

void BakePipeline::render(double dt) noexcept {
    Clock clk;
    stream() << display_shader_(frame_index(),
                                lightmap_base_index_)
                    .dispatch(resolution());
    stream() << synchronize();
    stream() << commit();
    double ms = clk.elapse_ms();
    Env::printer().retrieve_immediately();
    integrator()->accumulate_render_time(ms);
    printf("time consuming (current frame: %.3f, average: %.3f) frame index: %u    \r", ms, render_time() / frame_index(), frame_index());
}

void BakePipeline::display(double dt) noexcept {
    Pipeline::display(dt);
}

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BakePipeline)