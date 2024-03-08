//
// Created by Zero on 2024/3/8.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"
#include "utils.h"

namespace vision {

class SVGF;

class Modulate : public Context {
private:
    SVGF *_svgf{nullptr};
    using signature = void(Buffer<float4>, Buffer<float4>,
                           Buffer<SVGFData>, Buffer<float4> output);
    Shader<signature> _shader;

public:
    explicit Modulate(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision
