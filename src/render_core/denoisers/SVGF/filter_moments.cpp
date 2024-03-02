//
// Created by Zero on 2024/2/15.
//

#include "filter_moments.h"
#include "svgf.h"

namespace vision {

void FilterMoments::prepare() noexcept {
}

void FilterMoments::compile() noexcept {
    Kernel kernel = [&](BufferVar<SVGFData> svgf_buffer, BufferVar<PixelGeometry>,
                        BufferVar<float> history_buffer,
                        Float sigma_rt, Float sigma_normal) {
        Float history = history_buffer.read(dispatch_id());

        $if(history < 4) {
            $return();
        };
        SVGFDataVar svgf_data = svgf_buffer.read(dispatch_id());

        Float weight_sum_illumi = 0;
        Float3 sum_illumi = make_float3(0.f);
        Float2 sum_moments = make_float2(0.f);

        Float4 illumi_v = svgf_data.illumi_v;
        Float lumi = luminance(illumi_v.xyz());
        
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