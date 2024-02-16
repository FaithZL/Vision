//
// Created by Zero on 2024/2/8.
//

#include "svgf.h"

namespace vision {

void SVGF::prepare_buffers() {
    Pipeline *rp = pipeline();
    auto init_buffer = [&]<typename T>(Buffer<T> &buffer, uint num, const string &desc = "") {
        buffer = device().create_buffer<T>(num, desc);
        vector<T> vec;
        vec.assign(num, T{});
        buffer.upload_immediately(vec.data());
    };
    init_buffer(prev_normal_depth, rp->pixel_num(), "SVGF::prev_normal_depth");
    init_buffer(history, rp->pixel_num(), "SVGF::history");
    init_buffer(svgf_data, rp->pixel_num() * 2, "SVGF::svgf_data x 2");
    svgf_data.register_self(0, rp->pixel_num());
    svgf_data.register_self(rp->pixel_num(), rp->pixel_num());
}

uint SVGF::cur_svgf_index(ocarina::uint frame_index) const noexcept {
    return ((frame_index + 1) & 1) + svgf_data_base();
}

uint SVGF::prev_svgf_index(ocarina::uint frame_index) const noexcept {
    return ((frame_index) & 1) + svgf_data_base();
}

void SVGF::prepare() noexcept {
    prepare_buffers();
    _compute_gbuffer.prepare();
    _reproject.prepare();
    _filter_moment.prepare();
    _atrous.prepare();
}

void SVGF::compile() noexcept {
    _compute_gbuffer.compile();
    _reproject.compile();
    _filter_moment.compile();
    _atrous.compile();
}

CommandList SVGF::dispatch(vision::DenoiseInput &input) noexcept {
    CommandList ret;
    ret << _compute_gbuffer.dispatch(input);
    ret << _reproject.dispatch(input);
    ret << _filter_moment.dispatch(input);
    for (int i = 0; i < N; ++i) {
        uint step_width = 1 << i;
        ret << _atrous.dispatch(input, step_width);
    }
    return ret;
}

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SVGF)