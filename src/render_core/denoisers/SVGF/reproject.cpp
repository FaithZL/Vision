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
    Kernel kernel = [&](BufferVar<PixelData> pixel_data, BufferVar<float4> radiance,
                        Uint cur_index, Uint prev_index) {
        BindlessArrayBuffer<SVGFData> cur_data = pipeline()->buffer<SVGFData>(cur_index);
        BindlessArrayBuffer<SVGFData> prev_data = pipeline()->buffer<SVGFData>(prev_index);
    };
    _shader = device().compile(kernel, "SVGF-reproject");
}

CommandList Reproject::dispatch(vision::DenoiseInput &input) noexcept {
    CommandList ret;
    uint cur_index = ((input.frame_index + 1) & 1) + _svgf->svgf_data_base();
    uint prev_index = ((input.frame_index) & 1) + _svgf->svgf_data_base();
    ret << _shader(*input.pixel_data, *input.radiance, cur_index, prev_index).dispatch(input.resolution);
    return ret;
}

}// namespace vision