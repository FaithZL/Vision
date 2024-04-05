//
// Created by Zero on 2024/3/8.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"
#include "utils.h"

namespace vision::svgf {
struct ModulateParam {
    BufferProxy<float4> albedo_buffer;
    BufferProxy<float4> emission_buffer;
    BufferProxy<SVGFData> svgf_buffer;
    BufferProxy<float4> output_buffer;
};
}// namespace vision::svgf

OC_PARAM_STRUCT(vision::svgf::ModulateParam, albedo_buffer,
                emission_buffer, svgf_buffer, output_buffer){};

namespace vision::svgf {

class SVGF;

class Modulate : public Context {
private:
    SVGF *_svgf{nullptr};
    using signature = void(ModulateParam);
    Shader<signature> _shader;

public:
    explicit Modulate(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] ModulateParam construct_param(RealTimeDenoiseInput &input) const noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision::svgf
