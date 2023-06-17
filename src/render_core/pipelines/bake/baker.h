//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/bake_utlis.h"
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
        preprocess();
        prepare_geometry();
        compile_shaders();
        prepare_resource_array();
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
            _baked_shapes.emplace_back(item);
        });

        // uv spread
        std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
            UVSpreadResult spread_result;
            if (baked_shape.has_uv_cache()) {
                spread_result = baked_shape.load_uv_config_from_cache();
            } else {
                spread_result = _uv_spreader->apply(baked_shape.shape());
                baked_shape.save_to_cache(spread_result);
            }
            baked_shape.remedy_vertices(ocarina::move(spread_result));
        });

        // rasterize
        _rasterizer->compile_shader();
//        std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
//            baked_shape.prepare_for_rasterize();
////            pipeline()->stream() << synchronize();
//        });
        int i = 0;
        std::for_each(_baked_shapes.begin(), _baked_shapes.begin() + 7, [&](BakedShape &baked_shape) {
            baked_shape.prepare_for_rasterize();
            _rasterizer->apply(baked_shape, i++);
//            pipeline()->stream() << synchronize() << commit();
//            pipeline()->stream() << synchronize();
        });
        pipeline()->stream() << synchronize() << commit();
        Printer::instance().retrieve_immediately();
//        exit(0);
    }
};

}// namespace vision
