//
// Created by Zero on 2023/6/12.
//

#include "pipeline.h"

namespace vision {

BakerPipeline::BakerPipeline(const PipelineDesc &desc)
    : Pipeline(desc),
      _uv_spreader(Global::node_mgr().load<UVSpreader>(desc.uv_spreader_desc)),
      _rasterizer(Global::node_mgr().load<Rasterizer>(desc.rasterizer_desc)) {
    create_cache_directory_if_necessary();
}

void BakerPipeline::prepare() noexcept {
    auto pixel_num = resolution().x * resolution().y;
    _final_picture.reset_all(device(), pixel_num);
    _scene.prepare();
    image_pool().prepare();
    preprocess();
    prepare_geometry();
    compile_shaders();
    prepare_resource_array();
}

void BakerPipeline::preprocess() noexcept {
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
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        baked_shape.prepare_for_rasterize();
    });
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        if (baked_shape.has_rasterization_cache()) {
            baked_shape.load_rasterization_from_cache();
        } else {
            _rasterizer->apply(baked_shape);
        }
    });
    pipeline()->stream() << synchronize() << commit();
    std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
        if (!baked_shape.has_rasterization_cache()) {
            baked_shape.save_rasterization_to_cache();
        }
    });

    Printer::instance().retrieve_immediately();
}

void BakerPipeline::render(double dt) noexcept {
    Clock clk;
    _scene.integrator()->render();
    double ms = clk.elapse_ms();
    _total_time += ms;
    ++_frame_index;
    cerr << ms << "  " << _total_time / _frame_index << "  " << _frame_index << endl;
    Printer::instance().retrieve_immediately();
}

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BakerPipeline)