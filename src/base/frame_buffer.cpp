//
// Created by Zero on 2024/2/18.
//

#include "frame_buffer.h"
#include "mgr/pipeline.h"

namespace vision {
FrameBuffer::FrameBuffer(const vision::FrameBufferDesc &desc)
    : Node(desc), GBuffer(pipeline()->bindless_array()),
      _motion_vectors(pipeline()->bindless_array()),
      _hit_bsdfs(pipeline()->bindless_array()),
      _surfaces(pipeline()->bindless_array()) {}

uint FrameBuffer::pixel_num() const noexcept {
    return pipeline()->pixel_num();
}

uint2 FrameBuffer::resolution() const noexcept {
    return pipeline()->resolution();
}

void FrameBuffer::prepare() noexcept {
    init_buffer(GBuffer, "FrameBuffer::GBuffer", 2);
    init_buffer(_motion_vectors, "FrameBuffer::_motion_vectors");
}

void FrameBuffer::prepare_surfaces() noexcept {
    init_buffer(_surfaces, "FrameBuffer::_surfaces", 2);
}

void FrameBuffer::prepare_hit_bsdfs() noexcept {
    init_buffer(_hit_bsdfs, "FrameBuffer::_hit_bsdfs", 1);
}


}// namespace vision