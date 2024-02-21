//
// Created by Zero on 2024/2/18.
//

#include "base/frame_buffer.h"

namespace vision {

class RasterFrameBuffer : public FrameBuffer {
private:
public:
    explicit RasterFrameBuffer(const FrameBufferDesc &desc)
        : FrameBuffer(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void compile() noexcept override {
    }
    [[nodiscard]] CommandList compute_GBuffer(uint frame_index, BufferView<PixelGeometry> gbuffer,
                                              BufferView<float2> motion_vectors,
                                              BufferView<float4> albedo,
                                              BufferView<float4> emission) const noexcept override {
        return {};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RasterFrameBuffer)