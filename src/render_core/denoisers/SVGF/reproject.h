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
    using signature = void(Buffer<PixelGeometry>, Buffer<PixelGeometry>, Buffer<float>,
                           Buffer<float2>, Buffer<float4>,
                           Buffer<float4>, Buffer<float4>, float, float, uint, uint);
    Shader<signature> _shader;

public:
    explicit Reproject(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    [[nodiscard]] Bool is_valid_reproject(const OCPixelGeometry &cur, const OCPixelGeometry &prev) const noexcept;
    [[nodiscard]] Bool load_prev_data(const OCPixelGeometry &cur_geom, const BufferVar<PixelGeometry> &prev_gbuffer,
                                      const BufferVar<float> &history_buffer,
                                      const Float2 &motion_vec, const Uint &cur_buffer_index, const Uint &prev_buffer_index,
                                      Float *history, Float4 *prev_illumination,
                                      Float2 *prev_moments) const noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision