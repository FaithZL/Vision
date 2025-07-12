//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"
#include "utils.h"

namespace vision::svgf {

struct ReprojectParam {
public:
    BufferDesc<PixelGeometry> gbuffer;
    BufferDesc<PixelGeometry> prev_gbuffer;
    BufferDesc<float> history_buffer;
    BufferDesc<float2> motion_vectors;
    float alpha{};
    float moments_alpha{};
    uint history_limit{};
    BufferDesc<SVGFData> cur_buffer{};
    BufferDesc<SVGFData> prev_buffer{};
};

}// namespace vision::svgf

OC_PARAM_STRUCT(vision::svgf, ReprojectParam, gbuffer, prev_gbuffer,
                history_buffer, motion_vectors, alpha, moments_alpha,
                history_limit, cur_buffer, prev_buffer){};

namespace vision::svgf {

class SVGF;

class Reproject : public Context {
private:
    SVGF *svgf_{nullptr};
    using signature = void(ReprojectParam);
    Shader<signature> shader_;

public:
    explicit Reproject(SVGF *svgf)
        : svgf_(svgf) {}
    void prepare() noexcept;
    [[nodiscard]] Bool is_valid_reproject(const PixelGeometryVar &cur, const PixelGeometryVar &prev) const noexcept;
    [[nodiscard]] ReprojectParam construct_param(vision::RealTimeDenoiseInput &input) const noexcept;
    [[nodiscard]] Bool load_prev_data(const PixelGeometryVar &cur_geom, const BufferVar<PixelGeometry> &prev_gbuffer,
                                      const BufferVar<float> &history_buffer,
                                      const Float2 &motion_vec,const BufferVar<SVGFData> &prev_buffer,
                                      Float *history, Float3 *prev_illumination,
                                      Float2 *prev_moments) const noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision::svgf