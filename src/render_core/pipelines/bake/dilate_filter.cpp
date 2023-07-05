//
// Created by Zero on 2023/6/14.
//

#include "dilate_filter.h"

namespace vision {

using namespace ocarina;

void DilateFilter::compile() noexcept {
    Kernel kernel = [&](BufferVar<float4> positions, BufferVar<float4> normals,
                        BufferVar<float4> src, BufferVar<float4> dst) {
        Uint pixel_index = dispatch_id();
        Float4 position = positions.read(pixel_index);
        Float4 normal = normals.read(pixel_index);
        Float4 radiance = src.read(pixel_index);
        dst.write(pixel_index, radiance);

        Uint2 res = detail::decode(as<uint>(normal.w));
        Uint offset = as<uint>(position.w);
        Uint cur_index = pixel_index - offset;
        Uint2 pixel = make_uint2(cur_index % res.x, cur_index / res.x);

        auto in_bound = [&](const Int2 &p) -> Bool {
            return all(p >= 0) && all(p < make_int2(res));
        };

        auto is_interior = [&](const Uint &g_index) -> Bool {
            Float4 val = src.read(g_index);
            return val.w < 0.999f;
        };

        Uint exterior_num = 0;
        $if(is_interior(pixel_index)) {
            
        };
    };
    _shader = device().compile(kernel, "dilate filter");
}
}// namespace vision