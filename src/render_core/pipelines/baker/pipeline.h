//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/bake_utlis.h"
#include "expander.h"
#include "surface.h"

namespace vision {

class BakerPipeline : public Pipeline {
private:
    UVSpreader *_uv_spreader{};
    Rasterizer *_rasterizer{};
    UP<Expander> _expander;
    vector<BakedShape> _baked_shapes;
    UP<Surface> _surface;
    using transform_signature = void(Buffer<float4>,
                                     Buffer<float4>,
                                     float4x4 o2w);
    Shader<transform_signature> _transform_shader;

public:
    explicit BakerPipeline(const PipelineDesc &desc);
    static void create_cache_directory_if_necessary() {
        Context::create_directory_if_necessary(Global::instance().scene_cache_path());
    }
    void compile_shaders() noexcept override {
        _scene.integrator()->compile_shader();
    }
    void init_postprocessor(const vision::SceneDesc &scene_desc) override;
    void compile_transform_shader() noexcept;
    void init_scene(const vision::SceneDesc &scene_desc) override;
    void prepare() noexcept override;
    void render(double dt) noexcept override;
    void preprocess() noexcept override;
    template<typename Func>
    void for_each_need_bake(Func &&func) {
        auto &meshes = _scene.shapes();
        std::for_each(meshes.begin(), meshes.end(), [&](vision::Shape *item) {
            if (item->has_emission()) {
                return;
            }
            func(item);
        });
    }
};

}// namespace vision
