//
// Created by Zero on 2024/2/15.
//

#include "reproject.h"
#include "svgf.h"

namespace vision::svgf {
using namespace ocarina;

void Reproject::prepare() noexcept {
}

Bool Reproject::is_valid_reproject(const PixelGeometryVar &cur, const PixelGeometryVar &prev) const noexcept {
    Uint2 resolution = dispatch_dim().xy();
    Bool inside = in_screen(make_int2(prev.p_film), make_int2(resolution));

    Float z = cur.linear_depth;
    Float z_prev = prev.linear_depth;
    Bool z_valid = (abs(z - z_prev) / (cur.depth_gradient + 1e-2f)) < 10;

    Float3 normal = cur.normal.xyz();
    Float3 normal_prev = prev.normal.xyz();
    Bool normal_valid = (distance(normal, normal_prev) / (cur.normal.w + 1e-2f)) < 16;

    return inside && z_valid && normal_valid;
}

Bool Reproject::load_prev_data(const PixelGeometryVar &cur_geom, const BufferVar<PixelGeometry> &prev_gbuffer,
                               const BufferVar<float> &history_buffer,
                               const Float2 &motion_vec, const BufferVar<SVGFData> &prev_buffer,
                               Float *history, Float3 *prev_illumination,
                               Float2 *prev_moments) const noexcept {
    Uint2 pos = dispatch_idx().xy();

    Float2 prev_p_film = cur_geom.p_film - motion_vec;
    Int2 prev_pixel = make_int2(prev_p_film - 0.5f);
    Uint prev_pixel_index = dispatch_id(prev_pixel);
    Bool inside = in_screen(prev_pixel, make_int2(dispatch_dim().xy()));

    PixelGeometryVar prev_geom;

    *prev_illumination = make_float3(0);
    *prev_moments = make_float2(0);

    Bool valid = false;
    array<Bool, 4> v = {false, false, false, false};
    constexpr array<int2, 4> offsets = {int2(0, 0), int2(1, 0),
                                        int2(0, 1), int2(1, 1)};

    for (int i = 0; i < 4; ++i) {
        int2 ofs = offsets[i];
        Int2 loc = prev_pixel + ofs;
        Uint index = dispatch_id(loc);
        $if(in_screen(loc, make_int2(dispatch_dim().xy()))) {
            prev_geom = prev_gbuffer.read(index);
            v[i] = is_valid_reproject(cur_geom, prev_geom);
            valid = valid || v[i];
        };
    }

    $if(valid) {
        Float weight_sum = 0;
        Float x = fract(prev_p_film.x);
        Float y = fract(prev_p_film.y);

        /// bilinear weights
        array<Float, 4> weights = {(1 - x) * (1 - y), x * (1 - y), (1 - x) * y, x * y};

        for (int i = 0; i < 4; ++i) {
            int2 ofs = offsets[i];
            Int2 loc = prev_pixel + ofs;
            Uint index = dispatch_id(loc);
            $if(v[i] && in_screen(loc, make_int2(dispatch_dim().xy()))) {
                SVGFDataVar prev_svgf_data = prev_buffer.read(index);
                Float weight = weights[i];
                *prev_illumination += weight * prev_svgf_data->illumination();
                *prev_moments += weight * prev_svgf_data.moments;
                weight_sum += weight;
            };
        }
        valid = (weight_sum >= 0.01f);
        *prev_illumination = ocarina::select(valid, *prev_illumination / weight_sum, make_float3(0));
        *prev_moments = ocarina::select(valid, *prev_moments / weight_sum, make_float2(0));
    };

    $if(!valid) {
        Float valid_num = 0.f;
        foreach_neighbor(make_uint2(prev_pixel), [&](const Int2 &neighbor_pixel) {
            Uint index = dispatch_id(neighbor_pixel);
            PixelGeometryVar neighbor_data = prev_gbuffer.read(index);

            $if(is_valid_reproject(cur_geom, neighbor_data)) {
                SVGFDataVar prev_svgf_data = prev_buffer.read(index);
                *prev_illumination += prev_svgf_data->illumination();
                *prev_moments += prev_svgf_data.moments;
                valid_num += 1;
            };
        });

        $if(valid_num > 0) {
            valid = true;
            *prev_illumination /= valid_num;
            *prev_moments /= valid_num;
        };
    };

    $if(valid && inside) {
        *history = history_buffer.read(prev_pixel_index);
    }
    $else {
        *history = 0;
        *prev_illumination = make_float3(0);
        *prev_moments = make_float2(0);
    };

    return valid;
}

void Reproject::compile() noexcept {
    Kernel kernel = [&](Var<ReprojectParam> param) {
        PixelGeometryVar geom_data = param.gbuffer.read(dispatch_id());
        SVGFDataVar svgf_data = param.cur_buffer.read(dispatch_id());
        Float3 illumination = svgf_data->illumination();

        Float history = 0.f;
        Float3 prev_illumination = make_float3(0.f);
        Float2 prev_moments = make_float2(0.f);

        Float2 motion_vec = param.motion_vectors.read(dispatch_id());

        Bool valid = load_prev_data(geom_data, param.prev_gbuffer, param.history_buffer, motion_vec,
                                    param.prev_buffer,
                                    addressof(history), addressof(prev_illumination),
                                    addressof(prev_moments));

        history = min(cast<float>(param.history_limit), ocarina::select(valid, history + 1.0f, 1.0f));

        param.history_buffer.write(dispatch_id(), history);

        param.alpha = ocarina::select(valid, ocarina::max(param.alpha, rcp(history)), 1.f);
        param.moments_alpha = ocarina::select(valid, ocarina::max(param.moments_alpha, rcp(history)), 1.f);

        Float2 moments;
        moments.x = luminance(illumination);
        moments.y = sqr(moments.x);

        moments = lerp(make_float2(param.moments_alpha), prev_moments, moments);

        Float variance = max(0.f, moments.y - sqr(moments.x));
        illumination = lerp(make_float3(param.alpha), prev_illumination.xyz(), illumination);
        svgf_data.illumi_v = make_float4(illumination, variance);
        svgf_data.moments = moments;
        svgf_data.history = history;
        param.cur_buffer.write(dispatch_id(), svgf_data);
    };
    shader_ = device().compile(kernel, "SVGF-reproject");
}

ReprojectParam Reproject::construct_param(RealTimeDenoiseInput &input) const noexcept {
    ReprojectParam param;
    param.gbuffer = input.gbuffer.proxy();
    param.prev_gbuffer = input.prev_gbuffer.proxy();
    param.history_buffer = svgf_->history.proxy();
    param.motion_vectors = input.motion_vec.proxy();
    param.alpha = svgf_->alpha();
    param.moments_alpha = svgf_->moments_alpha();
    param.history_limit = svgf_->history_limit();
    param.cur_buffer = svgf_->cur_svgf_buffer(input.frame_index).proxy();
    param.prev_buffer = svgf_->prev_svgf_buffer(input.frame_index).proxy();
    return param;
}

CommandList Reproject::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    uint cur_index = svgf_->cur_svgf_index(input.frame_index);
    uint prev_index = svgf_->prev_svgf_index(input.frame_index);
    ret << shader_(construct_param(input))
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision::svgf