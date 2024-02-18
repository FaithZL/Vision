//
// Created by Zero on 2024/2/18.
//

#include "frame_buffer.h"
#include "mgr/pipeline.h"

namespace vision {
FrameBuffer::FrameBuffer(const vision::FrameBufferDesc &desc)
    : Node(desc), GBuffer(pipeline()->bindless_array()),
      _color_buffer(pipeline()->bindless_array()) {}

uint FrameBuffer::pixel_num() const noexcept {
    return pipeline()->pixel_num();
}

uint2 FrameBuffer::resolution() const noexcept {
    return pipeline()->resolution();
}

void FrameBuffer::prepare() noexcept {
    init_buffer(GBuffer, "FrameBuffer::GBuffer", 2);
    init_buffer(_motion_vec, "FrameBuffer::_motion_vec");
    init_buffer(_color_buffer, "FrameBuffer::_color_buffer");
    prepare_surface_buffer();
}

void FrameBuffer::prepare_surface_buffer() noexcept {
    init_buffer(_surface_buffer, "FrameBuffer::_surface_buffer", 2);
}

}// namespace vision