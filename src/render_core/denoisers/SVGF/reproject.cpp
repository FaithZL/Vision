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
    Kernel kernel = [&](BufferVar<PixelData> pixel_data, BufferVar<float4> radiance) {
        
    };
    _shader = device().compile(kernel, "SVGF-reproject");
}

CommandList Reproject::dispatch(vision::DenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(*input.pixel_data, *input.radiance).dispatch(input.resolution);
    return ret;
}

}// namespace vision