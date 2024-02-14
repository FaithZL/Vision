//
// Created by Zero on 2024/2/8.
//

#include "svgf.h"

namespace vision {

void SVGF::prepare_buffers() {
    Pipeline *rp = pipeline();
    auto init_buffer = [&]<typename T>(Buffer<T> &buffer, const string &desc = "") {
        buffer = device().create_buffer<T>(rp->pixel_num(), desc);
        vector<T> vec{rp->pixel_num(), T{}};
        buffer.upload_immediately(vec.data());
    };
    init_buffer(prev_depth_normal, "SVGF::prev_depth_normal");
    init_buffer(prev_filtered_lighting, "SVGF::prev_filtered_lighting");
    init_buffer(prev_moment, "SVGF::prev_moment");
}

void SVGF::prepare() noexcept {
    prepare_buffers();
    _reproject.prepare();
    _filter_moment.prepare();
    _atrous.prepare();
}

void SVGF::compile() noexcept {
    _reproject.compile();
    _filter_moment.compile();
    _atrous.compile();
}

CommandList SVGF::dispatch(vision::DenoiseInput &input) noexcept {
    CommandList ret;
    for (int i = 0; i < N; ++i) {
        uint step_width = pow(2, i);
        ret = _atrous.dispatch(input, step_width);
    }
    return ret;
}

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SVGF)