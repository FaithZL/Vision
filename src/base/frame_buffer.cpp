//
// Created by Zero on 2024/2/18.
//

#include "frame_buffer.h"
#include "mgr/pipeline.h"

namespace vision {
FrameBuffer::FrameBuffer(const vision::FrameBufferDesc &desc)
    : Node(desc) {}

uint FrameBuffer::pixel_num() const noexcept {
    return pipeline()->pixel_num();
}

uint2 FrameBuffer::resolution() const noexcept {
    return pipeline()->resolution();
}

BindlessArray &FrameBuffer::bindless_array() noexcept {
    return pipeline()->bindless_array();
}

BufferView<PixelGeometry> FrameBuffer::prev_gbuffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<PixelGeometry>(prev_gbuffer_index(frame_index));
}

BufferView<PixelGeometry> FrameBuffer::cur_gbuffer(ocarina::uint frame_index) const noexcept {
    return pipeline()->buffer_view<PixelGeometry>(cur_gbuffer_index(frame_index));
}

}// namespace vision