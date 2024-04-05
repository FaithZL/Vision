//
// Created by Zero on 2024/2/12.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"
#include "utils.h"

namespace vision::svgf {
struct FilterMomentsParam {
    BufferProxy<SVGFData> svgf_buffer;
    BufferProxy<PixelGeometry> gbuffer;
    BufferProxy<float> history_buffer;
    float sigma_rt{};
    float sigma_normal{};
};
}// namespace vision::svgf
OC_PARAM_STRUCT(vision::svgf::FilterMomentsParam, svgf_buffer,
                gbuffer, history_buffer, sigma_rt, sigma_normal) {};

namespace vision::svgf {

class SVGF;

class FilterMoments : public Context {
private:
    SVGF *_svgf{nullptr};
    using signature = void(FilterMomentsParam);
    Shader<signature> _shader;

public:
    explicit FilterMoments(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] FilterMomentsParam construct_param(RealTimeDenoiseInput &input) const noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision::svgf
