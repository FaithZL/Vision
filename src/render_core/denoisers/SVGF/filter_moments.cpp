//
// Created by Zero on 2024/2/15.
//

#include "filter_moments.h"
#include "svgf.h"

namespace vision {

void FilterMoments::prepare() noexcept {
}

void FilterMoments::compile() noexcept {
    Kernel kernel = [&](BufferVar<SVGFData> svgf_buffer, BufferVar<PixelGeometry> gbuffer,
                        BufferVar<float> history_buffer,
                        Float sigma_rt, Float sigma_normal) {
        Float history = history_buffer.read(dispatch_id());

        $if(history < 4) {
            $return();
        };

        OCPixelGeometry cur_goem = gbuffer.read(dispatch_id());
        $if(cur_goem.linear_depth < 0.f) {
            $return();
        };

        SVGFDataVar cur_svgf_data = svgf_buffer.read(dispatch_id());

        Float weight_sum_illumi = 0;
        Float3 sum_illumi = make_float3(0.f);
        Float2 sum_moments = make_float2(0.f);

        Float sigma_depth = max(cur_goem.depth_gradient, 1e-8f) * 3.f;

        Float cur_luminance = ocarina::luminance(cur_svgf_data->illumination());
        Int2 radius = make_int2(3);
        Int2 cur_pixel = make_int2(dispatch_idx().xy());

        foreach_neighbor(
            cur_pixel, [&](const Int2 &pixel) {
                Uint index = dispatch_id(pixel);
                SVGFDataVar svgf_data = svgf_buffer.read(index);
                OCPixelGeometry geom = gbuffer.read(index);
                Float3 illumination = svgf_data->illumination();
                Float2 moments = svgf_data.moments;
                Float luminance = ocarina::luminance(illumination);
                Float depth = geom.linear_depth;
                Float3 normal = geom.normal;

                Float weight = cal_weight(cur_goem.linear_depth, depth, sigma_depth,
                                          cur_goem.normal, normal, sigma_normal,
                                          cur_luminance, luminance, sigma_rt);

                
            },
            radius);
    };
    _shader = device().compile(kernel, "SVGF-filter_moments");
}

CommandList FilterMoments::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(_svgf->cur_svgf_buffer(input.frame_index), input.gbuffer,
                   _svgf->history, _svgf->sigma_rt(),
                   _svgf->sigma_normal())
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision