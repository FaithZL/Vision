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

Bool Reproject::is_neighbor(Int2 coord, Float depth, Float prev_depth, Float depth_fwidth, Float3 normal,
                            Float3 prev_normal, Float normal_fwidth) const noexcept {
    Bool ret = true;
    return ret;
}

Bool Reproject::load_prev_data(const OCPixelGeometry &geom_data, Float *history,
                               Float3 *prev_illumination, Float2 *prev_moments) const noexcept {
    Bool ret = true;
    Uint2 pos = dispatch_idx().xy();
//    Uint2 prev_pos = make_uint2(make_float2(pos) - motion_vec);

    Float3 normal = geom_data.normal.as_vec();
    Float depth = geom_data.linear_depth;
    Float depth_gradient = geom_data.depth_gradient;




    return ret;
}

void Reproject::compile() noexcept {
    Kernel kernel = [&](BufferVar<PixelGeometry> pixel_buffer, BufferVar<float4> radiance_buffer,
                        BufferVar<float4> albedo_buffer,
                        BufferVar<float4> emission_buffer,
                        Uint cur_index, Uint prev_index) {
        OCPixelGeometry geom_data = pixel_buffer.read(dispatch_id());
        Float3 emission = emission_buffer.read(dispatch_id()).xyz();
        Float3 albedo = albedo_buffer.read(dispatch_id()).xyz();
        Float3 radiance = radiance_buffer.read(dispatch_id()).xyz();
        Float3 illumination = demodulate(radiance.xyz() - emission, albedo);

        BindlessArrayBuffer<SVGFData> cur_data = pipeline()->buffer_var<SVGFData>(cur_index);
        BindlessArrayBuffer<SVGFData> prev_data = pipeline()->buffer_var<SVGFData>(prev_index);

        Float history = 0.f;
        Float3 prev_illumination = make_float3(0.f);
        Float2 prev_moments = make_float2(0.f);

        Bool valid = load_prev_data(geom_data, &history, &prev_illumination, &prev_moments);
    };
    _shader = device().compile(kernel, "SVGF-reproject");
}

CommandList Reproject::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    uint cur_index = _svgf->cur_svgf_index(input.frame_index);
    uint prev_index = _svgf->prev_svgf_index(input.frame_index);
    ret << _shader(input.gbuffer, input.radiance,
                   input.albedo, input.emission,
                   cur_index, prev_index)
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision