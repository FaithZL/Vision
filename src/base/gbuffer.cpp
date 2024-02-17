//
// Created by Zero on 2024/2/18.
//

#include "gbuffer.h"
#include "mgr/pipeline.h"

namespace vision {
GBuffer::GBuffer(const vision::GBufferDesc &desc)
    : Node(desc), _gbuffer0(pipeline()->bindless_array()),
      _gbuffer1(pipeline()->bindless_array()),
      _color_buffer(pipeline()->bindless_array()) {}
}// namespace vision