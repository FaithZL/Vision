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

    };
    _shader = device().compile(kernel, "SVGF-ComputeGBuffer");
}

CommandList ComputeGBuffer::dispatch(vision::DenoiseInput &input) noexcept {
    CommandList ret;
    ret << _shader(input.frame_index, *input.pixel_buffer).dispatch(pipeline()->resolution());
    return ret;
}

}// namespace vision