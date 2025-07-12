//
// Created by Zero on 2024/3/8.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"
#include "utils.h"

namespace vision::svgf {
struct ModulatorParam {
    BufferDesc<float4> albedo_buffer;
    BufferDesc<float4> emission_buffer;
    BufferDesc<SVGFData> svgf_buffer;
    BufferDesc<float4> radiance_buffer;
};
}// namespace vision::svgf

OC_PARAM_STRUCT(vision::svgf, ModulatorParam, albedo_buffer,
                emission_buffer, svgf_buffer, radiance_buffer){};

namespace vision::svgf {

class SVGF;

class Modulator : public Context {
private:
    SVGF *svgf_{nullptr};
    using signature = void(ModulatorParam);
    Shader<signature> modulate_;
    Shader<signature> demodulate_;

public:
    explicit Modulator(SVGF *svgf)
        : svgf_(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList modulate(RealTimeDenoiseInput &input) noexcept;
    [[nodiscard]] CommandList demodulate(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision::svgf
