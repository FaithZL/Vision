//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"
#include "render_graph/graph.h"

namespace vision {

class CustomizedRenderPipeline : public Pipeline {
private:
    RenderGraph render_graph_;

public:
    explicit CustomizedRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    void init_scene(const vision::SceneDesc &scene_desc) override {
        scene_.init(scene_desc);
        init_postprocessor(scene_desc.denoiser_desc);
    }

    void init_postprocessor(const DenoiserDesc &desc) override {
        postprocessor_.set_denoiser(Node::create_shared<Denoiser>(desc));
        postprocessor_.set_tone_mapper(scene_.sensor()->rad_collector()->tone_mapper());
    }

    void prepare_render_graph() noexcept override {
        SP<RenderPass> integrate = RenderPass::create("integrate");
        render_graph_.add_pass(integrate, "integrate");
        SP<RenderPass> accum = RenderPass::create("accumulate");
        render_graph_.add_pass(accum, "accumulate");
        SP<RenderPass> tonemapping = RenderPass::create("tonemapping");
        render_graph_.add_pass(tonemapping, "tonemapping");
        SP<RenderPass> gamma = RenderPass::create("gamma");
        render_graph_.add_pass(gamma, "gamma");

        render_graph_.add_edge("integrate.radiance", "accumulate.input");
        render_graph_.add_edge("accumulate.output", "tonemapping.input");
        render_graph_.add_edge("tonemapping.output", "gamma.input");
        render_graph_.mark_output("gamma.output");

        render_graph_.setup();
    }

    [[nodiscard]] const Buffer<float4> &view_buffer() override {
        return render_graph_.output_buffer();
    }

    void after_render() noexcept override {
        Env::debugger().reset_range();
        scene().sensor()->after_render();
        frame_buffer()->fill_window_buffer(render_graph_.output_buffer());
    }

    void prepare() noexcept override {
        Pipeline::prepare();
        auto pixel_num = resolution().x * resolution().y;
        final_picture_.reset_all(device(), pixel_num);
        scene_.prepare();
        image_pool().prepare();
        prepare_geometry();
        prepare_render_graph();
        upload_bindless_array();
        compile();
        preprocess();
    }

    void compile() noexcept override {
        Pipeline::compile();
        render_graph_.compile();
    }

    void render(double dt) noexcept override {
        stream() << render_graph_.dispatch() << synchronize() << commit();
        integrator()->increase_frame_index();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::CustomizedRenderPipeline)