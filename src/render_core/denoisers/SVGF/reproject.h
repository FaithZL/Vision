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

class Reproject : public Context {
private:
    SVGF *_svgf{nullptr};
    using signature = void(Buffer<PixelGeometry>, Buffer<float4>, uint, uint);
    Shader<signature> _shader;

public:
    explicit Reproject(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    [[nodiscard]] Bool is_neighbor(Int2 coord, Float depth, Float prev_depth, Float depth_fwidth, Float3 normal,
                                   Float3 prev_normal, Float normal_fwidth) const noexcept;
    [[nodiscard]] Bool load_prev_data(const OCPixelGeometry &geom_data, Float *history,
                                      Float3 *prev_illumination,
                                      Float2 *prev_moments) const noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(DenoiseInput &input) noexcept;
};

}// namespace vision