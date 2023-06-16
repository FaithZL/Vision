//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/bake.h"
#include "expander.h"

namespace vision {

class BakerPipeline : public Pipeline {
private:
    UVSpreader *_uv_spreader{};
    Rasterizer *_rasterizer{};
    UP<Expander> _expander;
    vector<BakedShape> _baked_shapes;

public:
    explicit BakerPipeline(const PipelineDesc &desc)
        : Pipeline(desc),
          _uv_spreader(Global::node_mgr().load<UVSpreader>(desc.uv_spreader_desc)),
          _rasterizer(Global::node_mgr().load<Rasterizer>(desc.rasterizer_desc)) {
        create_cache_directory_if_necessary();
    }

    static void create_cache_directory_if_necessary() {
        Context::create_directory_if_necessary(Global::instance().scene_cache_path());
    }

    void compile_shaders() noexcept override {
        _scene.integrator()->compile_shader();
        _rasterizer->compile_shader();
    }

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

    void render(double dt) noexcept override {
        Clock clk;
        _scene.integrator()->render();
        double ms = clk.elapse_ms();
        _total_time += ms;
        ++_frame_index;
        cerr << ms << "  " << _total_time / _frame_index << "  " << _frame_index << endl;
        Printer::instance().retrieve_immediately();
    }

    void preprocess() noexcept override {
        // fill baked shape list
        for_each_need_bake([&](Shape *item) {
            _baked_shapes.push_back(BakedShape(item));
        });

        // uv spread
        std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
            if (baked_shape.has_uv_cache()) {
                baked_shape.load_uv_spread_result_from_cache();
            } else {
                _uv_spreader->apply(baked_shape);
                baked_shape.save_uv_spread_result_to_cache();
            }
        });

        // raster
        std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
            baked_shape.allocate_device_memory();
            _rasterizer->apply(baked_shape);
        });
    }
};

}// namespace vision
