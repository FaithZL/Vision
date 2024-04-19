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
    VS_MAKE_PLUGIN_NAME_FUNC
    void compile() noexcept override {
    }

    CommandList compute_geom(ocarina::uint frame_index, BufferView<vision::PixelGeometry> gbuffer,
                             BufferView<ocarina::float2> motion_vectors,
                             BufferView<ocarina::float4> albedo,
                             BufferView<ocarina::float4> emission) const noexcept override {
        return {};
    }

    CommandList compute_grad(uint frame_index, BufferView<vision::PixelGeometry> gbuffer) const noexcept override {
        return {};
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