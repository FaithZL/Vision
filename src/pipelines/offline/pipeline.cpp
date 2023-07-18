//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class OfflineRenderPipeline : public Pipeline {
public:
    explicit OfflineRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}

    void prepare() noexcept override {
        auto pixel_num = resolution().x * resolution().y;
        _final_picture.reset_all(device(), pixel_num);
        _scene.prepare();
        image_pool().prepare();
        prepare_geometry();
        upload_resource_array();
        compile_shaders();
        preprocess();
    }

    void init_scene(const vision::SceneDesc &scene_desc) override {
        _scene.init(scene_desc);
        init_postprocessor(scene_desc);
    }

    void init_postprocessor(const vision::SceneDesc &scene_desc) override {
        _postprocessor.set_denoiser(_scene.load<Denoiser>(scene_desc.denoiser_desc));
        _postprocessor.set_tone_mapper(_scene.camera()->radiance_film()->tone_mapper());
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
        printf("time consuming (current frame: %.3f, average: %.3f) frame index: %u    \r", ms, _total_time / _frame_index, _frame_index);
        Printer::instance().retrieve_immediately();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OfflineRenderPipeline)