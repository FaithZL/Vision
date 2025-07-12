//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "base/sensor/filter.h"
#include "base/denoiser.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"
#include "utils.h"

namespace vision::svgf {
struct AtrousParam {
    BufferDesc<SVGFData> svgf_buffer;
    BufferDesc<PixelGeometry> gbuffer;
    BufferDesc<float> history_buffer;
    float sigma_rt{};
    float sigma_normal{};
    int step_size{};
};
}// namespace vision::svgf
OC_PARAM_STRUCT(vision::svgf, AtrousParam, svgf_buffer, gbuffer,
                history_buffer, sigma_rt, sigma_normal, step_size){};

namespace vision::svgf {
using namespace ocarina;

class SVGF;

class AtrousFilter : public Context {
private:
    SVGF *svgf_{nullptr};

    using signature = void(AtrousParam);
    Shader<signature> shader_;

public:
    explicit AtrousFilter(SVGF *svgf)
        : svgf_(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] AtrousParam construct_param(RealTimeDenoiseInput &input, uint step_width) const noexcept;
    [[nodiscard]] CommandList dispatch(vision::RealTimeDenoiseInput &input, uint step_width) noexcept;
};

}// namespace vision::svgf