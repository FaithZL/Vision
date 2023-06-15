//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/uv_spreader.h"
#include "base/rasterizer.h"
#include "expander.h"

namespace vision {

class BakerPipeline : public Pipeline {
private:
    UVSpreader *_uv_spreader{};
    Rasterizer *_rasterizer{};
    UP<Expander> _expander;

public:
    explicit BakerPipeline(const PipelineDesc &desc)
        : Pipeline(desc),
          _uv_spreader(Global::node_mgr().load<UVSpreader>(desc.uv_spreader_desc)),
          _rasterizer(Global::node_mgr().load<Rasterizer>(desc.rasterizer_desc)) {
    }

    template<typename Func>
    void for_each_need_bake(Func &&func) {
        auto &meshes = _scene.meshes();
        std::for_each(meshes.begin(), meshes.end(), [&](vision::Mesh *item) {
            if (!item->has_emission()) {
                func(item);
            }
        });
    }

    void preprocess() noexcept override {
        auto &meshes = _scene.meshes();
        for_each_need_bake([&](const auto &item) {
            _uv_spreader->apply(item);
        });
    }
};

}// namespace vision
