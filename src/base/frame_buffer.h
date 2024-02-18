//
// Created by Zero on 2024/2/17.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "scattering/interaction.h"

namespace vision {
using namespace ocarina;
struct PixelColor {
    array<float, 3> albedo{};
    array<float, 3> emission{};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision::PixelColor, albedo,emission) {};
// clang-format on

namespace vision {
using namespace ocarina;
struct PixelGeometry {
    array<float, 3> normal{};
    float normal_fwidth{};
    float depth_gradient{};
    float linear_depth{-1};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision::PixelGeometry, normal,
            normal_fwidth,depth_gradient,linear_depth) {};
// clang-format on
namespace vision {
using OCPixelGeometry = ocarina::Var<PixelGeometry>;
using OCPixelColor = ocarina::Var<PixelColor>;
}// namespace vision

namespace vision {

/// use for pingpong
[[nodiscard]] inline uint prev_index(uint frame_index) noexcept { return frame_index & 1; }
[[nodiscard]] inline uint cur_index(uint frame_index) noexcept { return (frame_index + 1) & 1; }

class FrameBuffer : public Node {
protected:
    /// save two frames of data
    RegistrableBuffer<PixelGeometry> GBuffer;

    /// save two frames of data , use for ReSTIR
    RegistrableBuffer<SurfaceData> _surface_buffer;

    RegistrableBuffer<float2> _motion_vec;

    RegistrableBuffer<PixelColor> _color_buffer;

public:
    using Desc = FrameBufferDesc;

public:
    explicit FrameBuffer(const FrameBufferDesc &desc);
    void prepare() noexcept override;
    [[nodiscard]] uint pixel_num() const noexcept;
    [[nodiscard]] uint2 resolution() const noexcept;
    [[nodiscard]] uint GBuffer_base() const noexcept { return GBuffer.index().hv(); }
    [[nodiscard]] uint surface_base() const noexcept { return _surface_buffer.index().hv(); }
    [[nodiscard]] uint prev_GBuffer_index(uint frame_index) const noexcept { return prev_index(frame_index) + GBuffer_base(); }
    [[nodiscard]] uint cur_GBuffer_index(uint frame_index) const noexcept { return cur_index(frame_index) + GBuffer_base(); }
    [[nodiscard]] uint cur_surface_index(uint frame_index) const noexcept { return cur_index(frame_index) + surface_base(); }
    [[nodiscard]] uint prev_surface_index(uint frame_index) const noexcept { return prev_index(frame_index) + surface_base(); }
    OC_MAKE_MEMBER_GETTER(motion_vec, &)
    void prepare_surface_buffer() noexcept;
    virtual void compile() noexcept = 0;
    template<typename T>
    void init_buffer(RegistrableBuffer<T> &buffer, const string &desc, uint count = 1) noexcept {
        uint element_num = count * pixel_num();
        buffer.super() = device().create_buffer<T>(element_num, desc);
        vector<T> vec{};
        vec.assign(element_num, T{});
        buffer.upload_immediately(vec.data());
        buffer.register_self();
        for (int i = 1; i < count; ++i) {
            buffer.register_view(pixel_num() * i, pixel_num());
        }
    }
};

}// namespace vision
