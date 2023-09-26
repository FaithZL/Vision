//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"
#include "render_graph/graph.h"

namespace vision {

class CustomizedRenderPipeline : public Pipeline {
private:
    RenderGraph _render_graph;

public:
    explicit CustomizedRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}

    void init_scene(const vision::SceneDesc &scene_desc) override {
        _scene.init(scene_desc);
        init_postprocessor(scene_desc.denoiser_desc);
    }

    void init_postprocessor(const DenoiserDesc &desc) override {
        _postprocessor.set_denoiser(_scene.load<Denoiser>(desc));
        _postprocessor.set_tone_mapper(_scene.camera()->radiance_film()->tone_mapper());
    }

    void prepare_render_graph() noexcept override {
        SP<RenderPass> integrate = RenderPass::create("integrate");
        _render_graph.add_pass(integrate, "integrate");
        SP<RenderPass> accum = RenderPass::create("accumulate");
        _render_graph.add_pass(accum, "accumulate");
        SP<RenderPass> tonemapping = RenderPass::create("tonemapping");
        _render_graph.add_pass(tonemapping, "tonemapping");
        SP<RenderPass> gamma = RenderPass::create("gamma");
        _render_graph.add_pass(gamma, "gamma");

        _render_graph.add_edge("integrate.radiance", "accumulate.input");
        _render_graph.add_edge("accumulate.output", "tonemapping.input");
        _render_graph.add_edge("tonemapping.output", "gamma.input");
        _render_graph.mark_output("gamma.output");

        _render_graph.setup();
    }

    [[nodiscard]] const Buffer<float4> &view_buffer() override {
        return _render_graph.output_buffer();
    }

    void prepare() noexcept override {
        auto pixel_num = resolution().x * resolution().y;
        _final_picture.reset_all(device(), pixel_num);
        _scene.prepare();
        image_pool().prepare();
        prepare_geometry();
        prepare_render_graph();
        upload_resource_array();
        compile();
        preprocess();
    }

    void compile() noexcept override {
        _render_graph.compile();
    }

    void render(double dt) noexcept override {
        stream() << _render_graph.dispatch() << synchronize() << commit();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::CustomizedRenderPipeline)