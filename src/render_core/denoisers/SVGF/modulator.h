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
struct ModulatorParam {
    BufferProxy<float4> albedo_buffer;
    BufferProxy<float4> emission_buffer;
    BufferProxy<SVGFData> svgf_buffer;
    BufferProxy<float4> radiance_buffer;
};
}// namespace vision::svgf

OC_PARAM_STRUCT(vision::svgf::ModulatorParam, albedo_buffer,
                emission_buffer, svgf_buffer, radiance_buffer){};

namespace vision::svgf {

class SVGF;

class Modulator : public Context {
private:
    SVGF *_svgf{nullptr};
    using signature = void(ModulatorParam);
    Shader<signature> _modulate;
    Shader<signature> _demodulate;

public:
    explicit Modulator(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList modulate(RealTimeDenoiseInput &input) noexcept;
    [[nodiscard]] CommandList demodulate(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision::svgf
