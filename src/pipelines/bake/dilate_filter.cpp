//
// Created by Zero on 2023/6/14.
//

#include "dilate_filter.h"
#include "baker.h"

namespace vision {

using namespace ocarina;

DilateFilter::DilateFilter(int padding)
    : padding_(padding) {}

void DilateFilter::compile() noexcept {
    Kernel kernel = [&](BufferVar<uint4> pixels,
                        BufferVar<float4> src, BufferVar<float4> dst) {
        Uint pixel_index = dispatch_id();
        Uint4 pixel_data = pixels.read(dispatch_id());
        Float4 radiance = src.read(pixel_index);

        Uint2 res = detail::uint_to_uint2(pixel_data.z);
        Uint offset = pixel_data.y;
        Uint cur_index = pixel_index - offset;
        Uint2 pixel = make_uint2(cur_index % res.x, cur_index / res.x);

        auto in_bound = [&](const Int2 &p) -> Bool {
            return all(p >= 0) && all(p < make_int2(res));
        };

        auto check = [&](const Uint &g_index) -> Bool {
            Float4 val = src.read(g_index);
            return val.w < 0.9f;
        };

        Uint exterior_num = 0;
        Float4 color = make_float4(0);
        $if(check(pixel_index)) {
            $for(x, -padding_, padding_ + 1) {
                $for(y, -padding_, padding_ + 1) {
                    Int2 p = make_int2(pixel) + make_int2(x, y);
                    Uint p_index = p.y * res.x + p.x;
                    Uint g_index = offset + p_index;
                    $if(!in_bound(p)) {
                        $continue;
                    };
                    Float4 val = src.read(g_index);
                    // todo val.w = 0.1f
                    $if(val.w > 0.f) {
                        color += val;
                        exterior_num += 1;
                    };
                };
            };
            $if(exterior_num != 0) {
                radiance = make_float4(color / cast<float>(exterior_num));
            };
        };
        dst.write(pixel_index, radiance);
    };
    _shader = device().compile(kernel, "dilate filter");
}
}// namespace vision