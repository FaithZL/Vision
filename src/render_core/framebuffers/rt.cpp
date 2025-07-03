//
// Created by Zero on 2024/2/18.
//

#include "base/frame_buffer.h"
#include "rhi/resources/shader.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class RayTracingFrameBuffer : public FrameBuffer {

public:
    RayTracingFrameBuffer() = default;
    explicit RayTracingFrameBuffer(const FrameBufferDesc &desc)
        : FrameBuffer(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(FrameBuffer, compute_geom_, compute_grad_, compute_hit_)


};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, RayTracingFrameBuffer)