//
// Created by Zero on 2024/2/18.
//

#include "base/frame_buffer.h"
#include "rhi/resources/shader.h"

namespace vision {

class RayTracingFrameBuffer : public FrameBuffer {
private:
    using signature = void(uint, Buffer<PixelGeometry>, Buffer<float4>, Buffer<float4>);
    Shader<signature> _shader;

public:
    explicit RayTracingFrameBuffer(const FrameBufferDesc &desc)
        : FrameBuffer(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    
    void compile() noexcept override {

    }

    [[nodiscard]] CommandList compute_GBuffer(uint frame_index, Buffer<PixelGeometry> &gbuffer,
                                              Buffer<float4> &albedo, Buffer<float4> &emission) const noexcept {
        CommandList ret;
        ret << _shader(frame_index, gbuffer, albedo, emission).dispatch(resolution());
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RayTracingFrameBuffer)