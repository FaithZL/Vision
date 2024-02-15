//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"

namespace vision {

class SVGF;

class Reproject : public Ctx {
private:
    SVGF *_svgf{nullptr};
    using signature = void(Buffer<PixelData>, Buffer<float4>, uint, uint);
    Shader<signature> _shader;

public:
    explicit Reproject(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(DenoiseInput &input) noexcept;
};

}// namespace vision