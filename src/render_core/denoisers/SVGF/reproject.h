//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"

namespace vision::svgf {

struct ReprojectParam {
public:
    BufferProxy<PixelGeometry> gbuffer;
    BufferProxy<PixelGeometry> prev_gbuffer;
    BufferProxy<float> history_buffer;
    BufferProxy<float2> motion_vectors;
    BufferProxy<float4> radiance_buffer;
    BufferProxy<float4> albedo_buffer;
    BufferProxy<float4> emission_buffer;
    float alpha{};
    float moments_alpha{};
    uint history_limit{};
    uint cur_index{};
    uint prev_index{};
};

}// namespace vision::svgf

OC_PARAM_STRUCT(vision::svgf::ReprojectParam, gbuffer, prev_gbuffer,
                history_buffer, motion_vectors, radiance_buffer, albedo_buffer,
                emission_buffer, alpha, moments_alpha, history_limit, cur_index, prev_index){};

namespace vision {

class SVGF;

class Reproject : public Context {
private:
    SVGF *_svgf{nullptr};
    using signature = void(Buffer<PixelGeometry>, Buffer<PixelGeometry>, Buffer<float>,
                           Buffer<float2>, Buffer<float4>,
                           Buffer<float4>, Buffer<float4>, float, float, uint, uint, uint);
    Shader<signature> _shader;

public:
    explicit Reproject(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    [[nodiscard]] Bool is_valid_reproject(const OCPixelGeometry &cur, const OCPixelGeometry &prev) const noexcept;
    [[nodiscard]] Bool load_prev_data(const OCPixelGeometry &cur_geom, const BufferVar<PixelGeometry> &prev_gbuffer,
                                      const BufferVar<float> &history_buffer,
                                      const Float2 &motion_vec, const Uint &cur_buffer_index, const Uint &prev_buffer_index,
                                      Float *history, Float3 *prev_illumination,
                                      Float2 *prev_moments) const noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision