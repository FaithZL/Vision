//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"
#include "render_graph/graph.h"

namespace vision {

class RealTimeRenderPipeline : public Pipeline {
private:
    RenderGraph _render_graph;

public:
    explicit RealTimeRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}

    void init_scene(const vision::SceneDesc &scene_desc) override {
        _scene.init(scene_desc);
        init_postprocessor(scene_desc);
    }

    void init_postprocessor(const vision::SceneDesc &scene_desc) override {
        _postprocessor.set_denoiser(_scene.load<Denoiser>(scene_desc.denoiser_desc));
        _postprocessor.set_tone_mapper(_scene.camera()->radiance_film()->tone_mapper());
    }

    void prepare_render_graph() noexcept override {
        RenderPass *integrate = RenderPass::create("integrate");
        _render_graph.add_pass(integrate, "integrate");
        RenderPass *accum = RenderPass::create("accumulate");
        _render_graph.add_pass(accum, "accumulate");
        RenderPass *tonemapping = RenderPass::create("tonemapping");
        _render_graph.add_pass(tonemapping, "tonemapping");
        RenderPass *gamma = RenderPass::create("gamma");
        _render_graph.add_pass(gamma, "gamma");

        _render_graph.add_edge("integrate.radiance", "accumulate.input");
        _render_graph.add_edge("accumulate.output", "tonemapping.input");
        _render_graph.add_edge("tonemapping.output", "gamma.input");
        _render_graph.mark_output("gamma.output");

        _render_graph.setup();
        int i = 0;
    }

    void prepare() noexcept override {
        auto pixel_num = resolution().x * resolution().y;
        _final_picture.reset_all(device(), pixel_num);
        _scene.prepare();
        image_pool().prepare();
        prepare_geometry();
        prepare_render_graph();
        prepare_resource_array();
        compile_shaders();
        preprocess();
    }

    void compile_shaders() noexcept override {
        _scene.integrator()->compile_shader();
    }

    void render(double dt) noexcept override {
        _scene.integrator()->render();
    }

    void display(double dt) noexcept override {
        Clock clk;
        render(dt);
        double ms = clk.elapse_ms();
        _total_time += ms;
        ++_frame_index;
        printf("time consuming (current frame: %f, average: %f) frame index: %u   \r", ms, _total_time / _frame_index, _frame_index);
        Printer::instance().retrieve_immediately();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeRenderPipeline)