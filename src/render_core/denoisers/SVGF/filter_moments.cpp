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
                        Float sigma_rt, Float sigma_normal) {
        $condition_info("{} {}  ", sigma_rt, sigma_normal);
    };
    _shader = device().compile(kernel, "SVGF-filter_moments");
}

CommandList FilterMoments::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(_svgf->cur_svgf_buffer(input.frame_index), input.gbuffer,
                   _svgf->sigma_rt(), _svgf->sigma_normal())
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision