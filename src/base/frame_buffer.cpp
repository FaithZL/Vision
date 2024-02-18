//
// Created by Zero on 2024/2/18.
//

#include "frame_buffer.h"
#include "mgr/pipeline.h"

namespace vision {
FrameBuffer::FrameBuffer(const vision::FrameBufferDesc &desc)
    : Node(desc), GBuffer(pipeline()->bindless_array()),
      _color_buffer(pipeline()->bindless_array()) {}

void FrameBuffer::prepare() noexcept {

}
}// namespace vision