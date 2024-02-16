//
// Created by Zero on 2024/2/16.
//

#include "compute_gbuffer.h"
#include "svgf.h"

namespace vision {

void ComputeGBuffer::prepare() noexcept {
}

void ComputeGBuffer::compile() noexcept {
    Kernel kernel = [&](Uint frame_index, BufferVar<PixelData> pixel_buffer) {
        Int2 radius = make_int2(1);
        Uint x_sample_num = 0u;
        Uint y_sample_num = 0u;
        Float3 normal_dx = make_float3(0.f);
        Float3 normal_dy = make_float3(0.f);

        Float depth_dx = 0.f;
        Float depth_dy = 0.f;

        Uint2 center = dispatch_idx().xy();
        OCPixelData center_data = pixel_buffer.read(dispatch_id());
        for_each_neighbor(radius, [&](const Int2 &pixel) {
            Uint index = dispatch_id(pixel);
            OCPixelData neighbor_data = pixel_buffer.read(index);
            $if(center.x > pixel.x) {
                x_sample_num += 1;
                normal_dx += center_data.ng.as_vec() - neighbor_data.ng.as_vec();
                depth_dx += center_data.linear_depth - neighbor_data.linear_depth;
            }
            $elif(pixel.x > center.x) {
                x_sample_num += 1;
                normal_dx += neighbor_data.ng.as_vec() - center_data.ng.as_vec();
                depth_dx += neighbor_data.linear_depth - center_data.linear_depth;
            };

            $if(center.y > pixel.y) {
                y_sample_num += 1;
                normal_dy += center_data.ng.as_vec() - neighbor_data.ng.as_vec();
                depth_dy += center_data.linear_depth - neighbor_data.linear_depth;
            }
            $elif(pixel.y > center.y) {
                y_sample_num += 1;
                normal_dy += neighbor_data.ng.as_vec() - center_data.ng.as_vec();
                depth_dy += neighbor_data.linear_depth - center_data.linear_depth;
            };
        });
        normal_dx /= cast<float>(x_sample_num);
        normal_dy /= cast<float>(y_sample_num);
        Float3 normal_fwidth = abs(normal_dx) + abs(normal_dy);
        center_data.normal_fwidth = length(normal_fwidth);

        depth_dx /= x_sample_num;
        depth_dy /= y_sample_num;
        center_data.depth_gradient = abs(depth_dx) + abs(depth_dy);
        pixel_buffer.write(dispatch_id(), center_data);
    };
    _shader = device().compile(kernel, "SVGF-ComputeGBuffer");
}

CommandList ComputeGBuffer::dispatch(vision::DenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(input.frame_index,
                   *input.pixel_buffer)
               .dispatch(pipeline()->resolution());
    return ret;
}

}// namespace vision