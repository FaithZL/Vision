//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class FixedRenderPipeline : public Pipeline {
public:
    explicit FixedRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    void prepare() noexcept override {
        Pipeline::prepare();
        auto pixel_num = resolution().x * resolution().y;
        final_picture_.reset_all(device(), pixel_num, "offline final picture");
        scene_.prepare();
        image_pool().prepare();
        prepare_geometry();
        upload_bindless_array();
        compile();
        preprocess();
    }

    void init_scene(const vision::SceneDesc &scene_desc) override {
        scene_.init(scene_desc);
        init_postprocessor(scene_desc.denoiser_desc);
    }

    void init_postprocessor(const DenoiserDesc &desc) override {
        postprocessor_.set_denoiser(Node::create_shared<Denoiser>(desc));
        postprocessor_.set_tone_mapper(scene_.sensor()->rad_collector()->tone_mapper());
    }

    void compile() noexcept override {
        Pipeline::compile();
        scene_.integrator()->compile();
    }

    void render(double dt) noexcept override {
        scene_.integrator()->render();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::FixedRenderPipeline)