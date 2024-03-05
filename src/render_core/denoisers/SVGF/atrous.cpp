//
// Created by Zero on 2024/2/15.
//

#include "atrous.h"
#include "svgf.h"

namespace vision {

void AtrousFilter::prepare() noexcept {
}

void AtrousFilter::compile() noexcept {

    auto compute_variance = [&](BufferVar<SVGFData> &svgf_buffer, const Int2 &pixel) {
        Float sum = 0.f;
        Var<array<float2, 2>> tab;
        tab[0] = make_float2(1.f / 4, 1.f / 8);
        tab[1] = make_float2(1.f / 8, 1.f / 16);

        foreach_neighbor(pixel, [&](const Int2 &neighbor) {
            Int2 delta = neighbor - pixel;
            Float weight = tab[abs(delta.x)][abs(delta.y)];
            Uint index = dispatch_id(neighbor);
            SVGFDataVar svgf_data = svgf_buffer.read(index);
            sum += svgf_data->variance() * weight;
        });
        return sum;
    };

    Kernel kernel = [&](BufferVar<SVGFData> svgf_buffer, BufferVar<PixelGeometry> gbufer,
                        BufferVar<float> history_buffer, Float sigma_rt, Float sigma_normal,
                        Int step_size) {
        float eps_variance = 1e-10f;
        constexpr array<float, 3> kernel_weights = {1.f, 2.f / 3, 1.f / 6};
        SVGFDataVar cur_svgf_data = svgf_buffer.read(dispatch_id());

        Float3 cur_illumination = cur_svgf_data->illumination();
        Float cur_luminance = ocarina::luminance(cur_illumination);
        Float variance = compute_variance(svgf_buffer, make_int2(dispatch_idx()));
        
        Float history = history_buffer.read(dispatch_id());
    };
    _shader = device().compile(kernel, "SVGF-atrous");
}

CommandList AtrousFilter::dispatch(vision::RealTimeDenoiseInput &input, ocarina::uint step_width) noexcept {
    CommandList ret;
    ret << _shader(_svgf->svgf_data, input.gbuffer, _svgf->history,
                   _svgf->sigma_rt(), _svgf->sigma_normal(), step_width)
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision