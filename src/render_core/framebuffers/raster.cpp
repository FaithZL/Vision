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
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RasterFrameBuffer)