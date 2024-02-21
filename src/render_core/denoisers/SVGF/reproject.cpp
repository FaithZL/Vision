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
    Bool inside = all(prev.p_film > 0.f) && all(prev.p_film < make_float2(resolution));

    Float z = cur.linear_depth;
    Float z_prev = prev.linear_depth;
    Bool z_valid = (abs(z - z_prev) / (cur.depth_gradient + 1e-2f)) < 10;

    Float3 normal = cur.normal.as_vec3();
    Float3 normal_prev = prev.normal.as_vec3();
    Bool normal_valid = (distance(normal, normal_prev) / (cur.normal_fwidth + 1e-2f)) < 16;

    return inside && z_valid && normal_valid;
}

Bool Reproject::load_prev_data(const OCPixelGeometry &geom_data, const BufferVar<PixelGeometry> &gbuffer,
                               const Float2 &motion_vec, Float *history, Float3 *prev_illumination,
                               Float2 *prev_moments) const noexcept {
    Bool ret = true;
    Uint2 pos = dispatch_idx().xy();

    Uint2 prev_pixel = make_uint2(geom_data.p_film - motion_vec);
    
    OCPixelGeometry prev_geom = gbuffer.read(dispatch_id(prev_pixel));

    ret = is_valid_reproject(geom_data, prev_geom);


    return ret;
}

void Reproject::compile() noexcept {
    Kernel kernel = [&](BufferVar<PixelGeometry> gbuffer,
                        BufferVar<float2> motion_vectors,
                        BufferVar<float4> radiance_buffer,
                        BufferVar<float4> albedo_buffer,
                        BufferVar<float4> emission_buffer,
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

        Bool valid = load_prev_data(geom_data, gbuffer, motion_vec, &history, &prev_illumination, &prev_moments);

        BindlessArrayBuffer<SVGFData> cur_data = pipeline()->buffer_var<SVGFData>(cur_index);
        BindlessArrayBuffer<SVGFData> prev_data = pipeline()->buffer_var<SVGFData>(prev_index);
    };
    _shader = device().compile(kernel, "SVGF-reproject");
}

CommandList Reproject::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    uint cur_index = _svgf->cur_svgf_index(input.frame_index);
    uint prev_index = _svgf->prev_svgf_index(input.frame_index);
    ret << _shader(input.gbuffer, input.motion_vec, input.radiance,
                   input.albedo, input.emission,
                   cur_index, prev_index)
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision