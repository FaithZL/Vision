//
// Created by Zero on 2024/2/15.
//

#include "atrous.h"
#include "svgf.h"

namespace vision::svgf {

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

    Kernel kernel = [&](Var<AtrousParam> param) {
        float eps_variance = 1e-10f;
        Float3 kernel_weights = make_float3(1.f, 2.f / 3, 1.f / 6);
        SVGFDataVar cur_svgf_data = param.svgf_buffer.read(dispatch_id());

        Float3 cur_illumination = cur_svgf_data->illumination();
        Float cur_luminance = ocarina::luminance(cur_illumination);
        Float variance = compute_variance(param.svgf_buffer, make_int2(dispatch_idx()));

        Float history = param.history_buffer.read(dispatch_id());

        PixelGeometryVar cur_geom = param.gbuffer.read(dispatch_id());

        Float depth = cur_geom.linear_depth;

        $if(depth < 0.f) {
            $return();
        };

        Float sigma_depth = max(cur_geom.depth_gradient, 1e-8f) * param.step_size;
        Float sigma_illumi = param.sigma_rt * sqrt(max(0.f, eps_variance + variance));

        Float weight_sum_illumi = 1.f;
        Float4 sum_illumi_v = cur_svgf_data.illumi_v;

        Int2 radius = make_int2(2);
        Int2 cur_pixel = make_int2(dispatch_idx().xy());
        foreach_neighbor(
            cur_pixel,
            [&](const Int2 &neighbor) {
                Int2 offset = cur_pixel - neighbor;
                Bool center = all(offset == 0);
                $if(center) {
                    syntax::continue_();
                };
                Float kernel_weight = kernel_weights[abs(offset.x)] * kernel_weights[abs(offset.y)];
                Int2 new_offset = offset * param.step_size;
                Int2 target_pixel = cur_pixel + new_offset;
                $if(!in_screen(target_pixel, make_int2(dispatch_dim().xy()))) {
                    syntax::continue_();
                };
                Uint index = dispatch_id(target_pixel);
                PixelGeometryVar neighbor_geom = param.gbuffer.read(index);
                SVGFDataVar neighbor_svgf_data = param.svgf_buffer.read(index);
                Float neighbor_luminance = ocarina::luminance(neighbor_svgf_data->illumination());

                Float weight = SVGF::cal_weight(cur_geom.linear_depth, neighbor_geom.linear_depth,
                                                sigma_depth * length(make_float2(offset)),
                                                cur_geom.normal_fwidth.xyz(), neighbor_geom.normal_fwidth.xyz(),
                                                param.sigma_normal,cur_luminance, neighbor_luminance, sigma_illumi);

                Float illumi_weight = weight * kernel_weight;
                weight_sum_illumi += illumi_weight;
                sum_illumi_v += make_float4(make_float3(illumi_weight), sqr(illumi_weight)) * neighbor_svgf_data.illumi_v;
            },
            radius);
        Float4 filtered_illumi_v = sum_illumi_v / make_float4(make_float3(weight_sum_illumi), sqr(weight_sum_illumi));
        cur_svgf_data.illumi_v = filtered_illumi_v;
        param.svgf_buffer.write(dispatch_id(), cur_svgf_data);
    };
    shader_ = device().compile(kernel, "SVGF-atrous");
}

AtrousParam AtrousFilter::construct_param(RealTimeDenoiseInput &input, uint step_width) const noexcept {
    AtrousParam param;
    param.svgf_buffer = svgf_->cur_svgf_buffer(input.frame_index).proxy();
    param.gbuffer = input.gbuffer.proxy();
    param.history_buffer = svgf_->history.proxy();
    param.sigma_rt = svgf_->sigma_rt();
    param.sigma_normal = svgf_->sigma_normal();
    param.step_size = step_width;
    return param;
}

CommandList AtrousFilter::dispatch(vision::RealTimeDenoiseInput &input, ocarina::uint step_width) noexcept {
    CommandList ret;
    ret << shader_(construct_param(input, step_width))
               .dispatch(input.resolution);
    return ret;
}

}// namespace vision::svgf