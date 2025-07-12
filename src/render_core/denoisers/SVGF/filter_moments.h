//
// Created by Zero on 2024/2/12.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"
#include "utils.h"

namespace vision::svgf {
struct FilterMomentsParam {
    BufferDesc<SVGFData> svgf_buffer;
    BufferDesc<PixelGeometry> gbuffer;
    BufferDesc<float> history_buffer;
    float sigma_rt{};
    float sigma_normal{};
    int radius{3};
};
}// namespace vision::svgf
OC_PARAM_STRUCT(vision::svgf, FilterMomentsParam, svgf_buffer,
                gbuffer, history_buffer, sigma_rt, sigma_normal,radius) {};

namespace vision::svgf {

class SVGF;

class FilterMoments : public Context {
private:
    SVGF *svgf_{nullptr};
    using signature = void(FilterMomentsParam);
    Shader<signature> shader_;

public:
    explicit FilterMoments(SVGF *svgf)
        : svgf_(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] FilterMomentsParam construct_param(RealTimeDenoiseInput &input) const noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision::svgf
