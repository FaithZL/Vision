//
// Created by Zero on 2024/2/15.
//

#include "filter_moments.h"
#include "svgf.h"

namespace vision::svgf {

void FilterMoments::prepare() noexcept {
}

void FilterMoments::compile() noexcept {
    Kernel kernel = [&](Var<FilterMomentsParam> param) {
        Float history = param.history_buffer.read(dispatch_id());
        $if(history < 4) {
            $return();
        };

        PixelGeometryVar cur_geom = param.gbuffer.read(dispatch_id());
        $if(cur_geom.linear_depth < 0.f) {
            $return();
        };

        SVGFDataVar cur_svgf_data = param.svgf_buffer.read(dispatch_id());

        Float sigma_depth = max(cur_geom.depth_gradient, 1e-8f) * 3.f;

        Float cur_luminance = ocarina::luminance(cur_svgf_data->illumination());
        Int2 radius = make_int2(param.radius);
        Int2 cur_pixel = make_int2(dispatch_idx().xy_());

        Float weight_sum_illumi = 0;
        Float3 sum_illumi = make_float3(0.f);
        Float2 sum_moments = make_float2(0.f);

        foreach_neighbor(
            cur_pixel, [&](const Int2 &pixel) {
                Uint index = dispatch_id(pixel);
                SVGFDataVar svgf_data = param.svgf_buffer.read(index);
                PixelGeometryVar geom = param.gbuffer.read(index);
                Float3 illumination = svgf_data->illumination();
                Float2 moments = svgf_data.moments;
                Float luminance = ocarina::luminance(illumination);
                Float depth = geom.linear_depth;
                Float3 normal = geom.normal_fwidth.xyz_();

                Float weight = SVGF::cal_weight(cur_geom.linear_depth, depth, sigma_depth,
                                                cur_geom.normal_fwidth.xyz_(), normal, param.sigma_normal,
                                                cur_luminance, luminance, param.sigma_rt);

                weight_sum_illumi += weight;
                sum_illumi += illumination * weight;
                sum_moments += moments * weight;
            },
            radius);

        weight_sum_illumi = max(weight_sum_illumi, 1e-6f);
        sum_illumi /= weight_sum_illumi;
        sum_moments /= weight_sum_illumi;

        Float variance = sum_moments.y - sqr(sum_moments.x);
        variance *= 4.f / history;
        cur_svgf_data.illumi_v = make_float4(sum_illumi, variance);
        param.svgf_buffer.write(dispatch_id(), cur_svgf_data);
    };
    _shader = device().compile(kernel, "SVGF-filter_moments");
}

FilterMomentsParam FilterMoments::construct_param(RealTimeDenoiseInput &input) const noexcept {
    FilterMomentsParam param;
    param.svgf_buffer = svgf_->cur_svgf_buffer(input.frame_index).proxy();
    param.gbuffer = input.gbuffer.proxy();
    param.history_buffer = svgf_->history.proxy();
    param.sigma_rt = svgf_->sigma_rt();
    param.sigma_normal = svgf_->sigma_normal();
    param.radius = svgf_->moments_filter_radius();
    return param;
}

CommandList FilterMoments::dispatch(vision::RealTimeDenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(construct_param(input))
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision::svgf