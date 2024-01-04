//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class FixedRenderPipeline : public Pipeline {
public:
    explicit FixedRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare() noexcept override {
        auto pixel_num = resolution().x * resolution().y;
        _final_picture.reset_all(device(), pixel_num, "offline final picture");
        _scene.prepare();
        image_pool().prepare();
        prepare_geometry();
        upload_bindless_array();
        compile();
        preprocess();
    }

    void init_scene(const vision::SceneDesc &scene_desc) override {
        _scene.init(scene_desc);
        init_postprocessor(scene_desc.denoiser_desc);
    }

    void init_postprocessor(const DenoiserDesc &desc) override {
        _postprocessor.set_denoiser(_scene.load<Denoiser>(desc));
        _postprocessor.set_tone_mapper(_scene.camera()->radiance_film()->tone_mapper());
    }

    void compile() noexcept override {
        _scene.integrator()->compile();
    }

    void render(double dt) noexcept override {
        _scene.integrator()->render();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::FixedRenderPipeline)