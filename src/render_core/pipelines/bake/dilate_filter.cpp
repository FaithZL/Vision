//
// Created by Zero on 2023/6/14.
//

#include "dilate_filter.h"

namespace vision {

using namespace ocarina;

void DilateFilter::compile() noexcept {
    Kernel kernel = [&](BufferVar<float4> positions, BufferVar<float4> normals,
                        BufferVar<float4> src, BufferVar<float4> dst) {
        Float4 radiance = src.read(dispatch_id());
        dst.write(dispatch_id(), radiance);
    };
    _shader = device().compile(kernel, "dilate filter");
}
}// namespace vision