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
    UVSpreader * _uv_spreader{};
    Rasterizer *_rasterizer{};
    UP<Expander> _expander;

public:
    explicit BakerPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {
//        UVSpreaderDesc spreader_desc;
//        spreader_desc.sub_type = "xatlas";
        _uv_spreader = Global::node_mgr().load<UVSpreader>(desc.uv_spreader_desc);
        _rasterizer = Global::node_mgr().load<Rasterizer>(desc.rasterizer_desc);
    }

    void preprocess() noexcept override {
    }
};

}// namespace vision
