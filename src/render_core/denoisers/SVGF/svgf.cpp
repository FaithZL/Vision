//
// Created by Zero on 2024/2/8.
//

#include "svgf.h"

namespace vision::svgf {

void SVGF::prepare_buffers() {
    Pipeline *rp = pipeline();
    auto init_buffer = [&]<typename T>(Buffer<T> &buffer, uint num, const string &desc = "") {
        buffer = device().create_buffer<T>(num, desc);
        vector<T> vec;
        vec.assign(num, T{});
        buffer.upload_immediately(vec.data());
    };
    init_buffer(history, rp->pixel_num(), "SVGF::history");
    init_buffer(svgf_data, rp->pixel_num() * 2, "SVGF::svgf_data x 2");
    svgf_data.register_self(0, rp->pixel_num());
    svgf_data.register_view(rp->pixel_num(), rp->pixel_num());
}

void SVGF::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    changed_ |= widgets->check_box("turn on", addressof(switch_));
    changed_ |= widgets->check_box("reproject", addressof(reproject_switch_));
    changed_ |= widgets->check_box("filter moment", addressof(moment_filter_switch_));
    changed_ |= widgets->input_uint_limit("N", &N, 0, 10);
    changed_ |= widgets->input_float_limit("alpha", &alpha_, 0,
                                           1, 0.01, 0.05);
    changed_ |= widgets->input_float_limit("moments_alpha", &moments_alpha_,
                                           0, 1, 0.01, 0.05);
    changed_ |= widgets->input_uint_limit("history_limit", &history_limit_,
                                          1, 100, 1, 5);
    changed_ |= widgets->input_int_limit("moments_filter_radius", &moments_filter_radius_,
                                         0, 5, 1, 1);
    changed_ |= widgets->input_float_limit("sigma_rt", &sigma_rt_,
                                           0.01, 1e10, 1, 3);
    changed_ |= widgets->input_float_limit("sigma_normal", &sigma_normal_,
                                           0.01, 1e10, 1, 3);
}

uint SVGF::cur_svgf_index(ocarina::uint frame_index) const noexcept {
    return cur_index(frame_index) + svgf_data_base();
}

uint SVGF::prev_svgf_index(ocarina::uint frame_index) const noexcept {
    return prev_index(frame_index) + svgf_data_base();
}

BufferView<SVGFData> SVGF::cur_svgf_buffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<SVGFData>(cur_svgf_index(frame_index));
}

BufferView<SVGFData> SVGF::prev_svgf_buffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<SVGFData>(prev_svgf_index(frame_index));
}

Float SVGF::cal_weight(const Float &cur_depth, const Float &neighbor_depth, const Float &sigma_depth,
                       const Float3 &cur_normal, const Float3 &neighbor_normal, const Float &sigma_normal,
                       const Float &cur_illumi, const Float &neighbor_illumi, const Float &sigma_illumi) noexcept {
    Float ret = 0;
    Float weight_normal = pow(saturate(dot(cur_normal, neighbor_normal)), sigma_normal);
    Float weight_depth = ocarina::select(sigma_depth == 0, 0.f, abs(cur_depth - neighbor_depth) / sigma_depth);
    Float weight_illumi = abs(cur_illumi - neighbor_illumi) / sigma_illumi;
    ret = exp(-(max(weight_illumi, 0.f) + max(weight_depth, 0.f))) * weight_normal;
    return ret;
}

void SVGF::prepare() noexcept {
    prepare_buffers();
    reproject_.prepare();
    filter_moments_.prepare();
    atrous_.prepare();
    modulator_.prepare();
}

void SVGF::compile() noexcept {
    reproject_.compile();
    filter_moments_.compile();
    atrous_.compile();
    modulator_.compile();
}

CommandList SVGF::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    if (switch_) {
        ret << modulator_.demodulate(input);
        if (reproject_switch_) {
            ret << reproject_.dispatch(input);
        }
        if (moment_filter_switch_) {
            ret << filter_moments_.dispatch(input);
        }
        for (int i = 0; i < N; ++i) {
            uint step_width = 1 << i;
            ret << atrous_.dispatch(input, step_width);
        }
        ret << modulator_.modulate(input);
    }
    return ret;
}

}// namespace vision::svgf

VS_MAKE_CLASS_CREATOR_HOTFIX(vision::svgf, SVGF)