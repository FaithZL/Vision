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

void Reproject::compile() noexcept {
    Kernel kernel = [&](BufferVar<PixelData> pixel_buffer, BufferVar<float4> radiance_buffer,
                        Uint cur_index, Uint prev_index) {
        OCPixelData pixel_data = pixel_buffer.read(dispatch_id());
        Float4 radiance = radiance_buffer.read(dispatch_id());
        Float3 illumination = demodulate(radiance.xyz() - pixel_data.emission.as_vec(), pixel_data.albedo.as_vec());

        BindlessArrayBuffer<SVGFData> cur_data = pipeline()->buffer<SVGFData>(cur_index);
        BindlessArrayBuffer<SVGFData> prev_data = pipeline()->buffer<SVGFData>(prev_index);

        Float history = 0.f;
        Float3 prev_illumination = make_float3(0.f);
        Float2 prev_moments = make_float2(0.f);

        Bool valid = false;


    };
    _shader = device().compile(kernel, "SVGF-reproject");
}

CommandList Reproject::dispatch(vision::DenoiseInput &input) noexcept {
    CommandList ret;
    uint cur_index = _svgf->cur_svgf_index(input.frame_index);
    uint prev_index = _svgf->prev_svgf_index(input.frame_index);
    ret << _shader(*input.pixel_buffer, *input.radiance, cur_index, prev_index).dispatch(input.resolution);
    return ret;
}

}// namespace vision