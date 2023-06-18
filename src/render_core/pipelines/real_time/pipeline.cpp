//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class RealTimeRenderPipeline : public Pipeline {
public:
    explicit RealTimeRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}

    void prepare() noexcept override {
        auto pixel_num = resolution().x * resolution().y;
        _final_picture.reset_all(device(), pixel_num);
        _scene.prepare();
        image_pool().prepare();
        prepare_geometry();
        prepare_resource_array();
        compile_shaders();
        preprocess();
    }

    void compile_shaders() noexcept override {
        _scene.integrator()->compile_shader();
    }

    void render(double dt) noexcept override {
        Clock clk;
        _scene.integrator()->render();
        double ms = clk.elapse_ms();
        _total_time += ms;
        ++_frame_index;
        cerr << ms << "  " << _total_time / _frame_index << "  " << _frame_index << endl;
        Printer::instance().retrieve_immediately();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeRenderPipeline)