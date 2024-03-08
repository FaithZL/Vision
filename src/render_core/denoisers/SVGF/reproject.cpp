//
// Created by Zero on 2024/2/15.
//

#include "reproject.h"
#include "svgf.h"

namespace vision {
using namespace ocarina;
template<EPort p = D>
[[nodiscard]] oc_float3<p> demodulate_impl(const oc_float3<p> &c,
                                           const oc_float3<p> &albedo) {
    return c / ocarina::max(albedo, make_float3(0.001f, 0.001f, 0.001f));
}
VS_MAKE_CALLABLE(demodulate)

void Reproject::prepare() noexcept {
}

Bool Reproject::is_valid_reproject(const OCPixelGeometry &cur, const OCPixelGeometry &prev) const noexcept {
    Uint2 resolution = dispatch_dim().xy();
    Bool inside = in_screen(make_int2(prev.p_film), make_int2(resolution));

    Float z = cur.linear_depth;
    Float z_prev = prev.linear_depth;
    Bool z_valid = (abs(z - z_prev) / (cur.depth_gradient + 1e-2f)) < 10;

    Float3 normal = cur.normal;
    Float3 normal_prev = prev.normal;
    Bool normal_valid = (distance(normal, normal_prev) / (cur.normal_fwidth + 1e-2f)) < 16;

    return inside && z_valid && normal_valid;
}

Bool Reproject::load_prev_data(const OCPixelGeometry &cur_geom, const BufferVar<PixelGeometry> &prev_gbuffer,
                               const BufferVar<float> &history_buffer,
                               const Float2 &motion_vec, const Uint &cur_buffer_index, const Uint &prev_buffer_index,
                               Float *history, Float3 *prev_illumination,
                               Float2 *prev_moments) const noexcept {
    Uint2 pos = dispatch_idx().xy();

    Int2 prev_pixel = make_int2(cur_geom.p_film - motion_vec);
    Uint prev_pixel_index = dispatch_id(prev_pixel);
    Bool inside = in_screen(prev_pixel, make_int2(dispatch_dim().xy()));

    OCPixelGeometry prev_geom;

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

    BindlessArrayBuffer<SVGFData> prev_data = pipeline()->buffer_var<SVGFData>(prev_buffer_index);

    $if(valid) {
        Float weight_sum = 0;
        Float x = fract(prev_geom.p_film.x);
        Float y = fract(prev_geom.p_film.y);

        /// bilinear weights
        array<Float, 4> weights = {(1 - x) * (1 - y), x * (1 - y), (1 - x) * y, x * y};

        for (int i = 0; i < 4; ++i) {
            int2 ofs = offsets[i];
            Int2 loc = prev_pixel + ofs;
            Uint index = dispatch_id(loc);
            $if(v[i] && in_screen(loc, make_int2(dispatch_dim().xy()))) {
                SVGFDataVar prev_svgf_data = prev_data.read(index);
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
            OCPixelGeometry neighbor_data = prev_gbuffer.read(index);

            $if(is_valid_reproject(cur_geom, neighbor_data)) {
                SVGFDataVar prev_svgf_data = prev_data.read(index);
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
    $condition_info("prev {} {} {}         ------------", *prev_illumination);

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
    Kernel kernel = [&](BufferVar<PixelGeometry> gbuffer,
                        BufferVar<PixelGeometry> prev_gbuffer,
                        BufferVar<float> history_buffer,
                        BufferVar<float2> motion_vectors,
                        BufferVar<float4> radiance_buffer,
                        BufferVar<float4> albedo_buffer,
                        BufferVar<float4> emission_buffer,
                        Float alpha, Float moments_alpha,
                        Uint history_limit,
                        Uint cur_index, Uint prev_index) {
        OCPixelGeometry geom_data = gbuffer.read(dispatch_id());

        Float3 emission = emission_buffer.read(dispatch_id()).xyz();
        Float3 albedo = albedo_buffer.read(dispatch_id()).xyz();
        Float3 radiance = radiance_buffer.read(dispatch_id()).xyz();
        Float3 illumination = demodulate(radiance.xyz() - emission, albedo);

        Float history = 0.f;
        Float3 prev_illumination = make_float3(0.f);
        Float2 prev_moments = make_float2(0.f);

        Float2 motion_vec = motion_vectors.read(dispatch_id());

        $condition_info("cur {} {} {}      ------------", illumination);

        Bool valid = load_prev_data(geom_data, prev_gbuffer, history_buffer, motion_vec,
                                    cur_index, prev_index,
                                    addressof(history), addressof(prev_illumination),
                                    addressof(prev_moments));

        history = min(cast<float>(history_limit), ocarina::select(valid, history + 1.0f, 1.0f));

        history_buffer.write(dispatch_id(), history);

        alpha = ocarina::select(valid, ocarina::max(alpha, rcp(history)), 1.f);
        moments_alpha = ocarina::select(valid, ocarina::max(moments_alpha, rcp(history)), 1.f);

        Float2 moments;
        moments.x = luminance(illumination);
        moments.y = sqr(moments.x);

        moments = lerp(make_float2(moments_alpha), prev_moments, moments);

        Float variance = max(0.f, moments.x - sqr(moments.y));
        SVGFDataVar svgf_data;

        BindlessArrayBuffer<SVGFData> cur_buffer = pipeline()->buffer_var<SVGFData>(cur_index);
        illumination = lerp(make_float3(alpha), prev_illumination.xyz(), illumination);
        svgf_data.illumi_v = make_float4(illumination, variance);
        svgf_data.moments = moments;
        svgf_data.history = history;

        cur_buffer.write(dispatch_id(), svgf_data);
    };
    _shader = device().compile(kernel, "SVGF-reproject");
}

CommandList Reproject::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    uint cur_index = _svgf->cur_svgf_index(input.frame_index);
    uint prev_index = _svgf->prev_svgf_index(input.frame_index);
    ret << _shader(input.gbuffer, input.prev_gbuffer, _svgf->history,
                   input.motion_vec, input.radiance,
                   input.albedo, input.emission,
                   _svgf->alpha(), _svgf->moments_alpha(),
                   _svgf->history_limit(),
                   cur_index, prev_index)
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision