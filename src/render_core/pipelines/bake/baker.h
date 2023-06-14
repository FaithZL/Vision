//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/uv_spreader.h"
#include "rasterizer.h"
#include "expander.h"

namespace vision {

class BakerPipeline : public Pipeline {
private:
    UVSpreader * _uv_spreader{};
    UP<Rasterizer> _rasterizer;
    UP<Expander> _expander;

public:
    explicit BakerPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}

    void preprocess() noexcept override {
    }
};

}// namespace vision
