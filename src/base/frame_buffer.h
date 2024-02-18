//
// Created by Zero on 2024/2/17.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "scattering/interaction.h"

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
}// namespace vision

namespace vision {

/// use for pingpong
template<typename T>
requires is_integral_expr_v<T>
[[nodiscard]] inline T prev_index(const T &frame_index) noexcept { return frame_index & 1; }
template<typename T>
requires is_integral_expr_v<T>
[[nodiscard]] inline T cur_index(const T &frame_index) noexcept { return (frame_index + 1) & 1; }

class FrameBuffer : public Node {
protected:
    /// save two frames of data
    RegistrableBuffer<PixelGeometry> GBuffer{};

    /// save two frames of data , use for ReSTIR
    RegistrableBuffer<SurfaceData> _surfaces{};

    RegistrableBuffer<HitBSDF> _hit_bsdfs{};

    RegistrableBuffer<float2> _motion_vectors{};

    RegistrableBuffer<float3> _bufferA;
    RegistrableBuffer<float3> _bufferB;

public:
    using Desc = FrameBufferDesc;

public:
    explicit FrameBuffer(const FrameBufferDesc &desc);
    [[nodiscard]] uint pixel_num() const noexcept;
    [[nodiscard]] uint2 resolution() const noexcept;
    [[nodiscard]] uint GBuffer_base() const noexcept { return GBuffer.index().hv(); }
    [[nodiscard]] uint surface_base() const noexcept { return _surfaces.index().hv(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T prev_GBuffer_index(const T &frame_index) const noexcept { return prev_index(frame_index) + GBuffer_base(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T cur_GBuffer_index(const T &frame_index) const noexcept { return cur_index(frame_index) + GBuffer_base(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T cur_surfaces_index(const T &frame_index) const noexcept { return cur_index(frame_index) + surface_base(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T prev_surfaces_index(const T &frame_index) const noexcept { return prev_index(frame_index) + surface_base(); }

    OC_MAKE_MEMBER_GETTER(motion_vectors, &)
    OC_MAKE_MEMBER_GETTER(surfaces, &)
    OC_MAKE_MEMBER_GETTER(hit_bsdfs, &)
    OC_MAKE_MEMBER_GETTER(bufferA, &)
    OC_MAKE_MEMBER_GETTER(bufferB, &)

#define VS_MAKE_PREPARE(buffer_name, count)                            \
    void prepare##buffer_name() noexcept {                             \
        init_buffer(buffer_name, "FrameBuffer::" #buffer_name, count); \
    }

    VS_MAKE_PREPARE(_surfaces, 2)
    VS_MAKE_PREPARE(GBuffer, 2)
    VS_MAKE_PREPARE(_hit_bsdfs, 1)
    VS_MAKE_PREPARE(_motion_vectors, 1)
    VS_MAKE_PREPARE(_bufferA, 1)
    VS_MAKE_PREPARE(_bufferB, 1)

#undef VS_MAKE_PREPARE

    [[nodiscard]] BindlessArray &bindless_array() noexcept;

    virtual void compile() noexcept = 0;
    template<typename T>
    void init_buffer(RegistrableBuffer<T> &buffer, const string &desc, uint count = 1) noexcept {
        uint element_num = count * pixel_num();
        buffer.super() = device().create_buffer<T>(element_num, desc);
        vector<T> vec{};
        vec.assign(element_num, T{});
        buffer.upload_immediately(vec.data());
        buffer.set_bindless_array(bindless_array());
        buffer.register_self();
        for (int i = 1; i < count; ++i) {
            buffer.register_view(pixel_num() * i, pixel_num());
        }
    }
};

}// namespace vision
