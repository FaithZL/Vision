//
// Created by Zero on 2024/2/17.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "sensor/camera.h"
#include "scattering/interaction.h"

namespace vision {
using namespace ocarina;
struct PixelGeometry {
    float4 normal_fwidth{};
    float2 p_film{};
    float depth_gradient{};
    float linear_depth{-1};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision,PixelGeometry, normal_fwidth, p_film,
            depth_gradient, linear_depth) {
    void set_normal(const Float3 &n) noexcept {
        normal_fwidth = make_float4(n, normal_fwidth.w);
    }

    void set_normal_fwidth(const Float &fw) noexcept {
        normal_fwidth.w = fw;
    }
};

// clang-format on
namespace vision {
using PixelGeometryVar = ocarina::Var<PixelGeometry>;
}// namespace vision

namespace vision {

/// use for pingpong
template<typename T>
requires is_integral_expr_v<T>
[[nodiscard]] inline T prev_index(const T &frame_index) noexcept { return frame_index & 1; }
template<typename T>
requires is_integral_expr_v<T>
[[nodiscard]] inline T cur_index(const T &frame_index) noexcept { return (frame_index + 1) & 1; }

template<typename TPixel, typename Func>
requires is_general_vector_v<remove_device_t<TPixel>>
void foreach_neighbor(const TPixel &pixel, Func func, const Int2 &radius = make_int2(1)) {
    Int2 cur_pixel = make_int2(pixel);
    Int2 res = make_int2(dispatch_dim().xy());
    Int x_start = cur_pixel.x - radius.x;
    x_start = max(0, x_start);
    Int x_end = cur_pixel.x + radius.x;
    x_end = min(x_end, res.x - 1);
    Int y_start = cur_pixel.y - radius.y;
    y_start = max(0, y_start);
    Int y_end = cur_pixel.y + radius.y;
    y_end = min(y_end, res.y - 1);
    $for(x, x_start, x_end + 1) {
        $for(y, y_start, y_end + 1) {
            func(make_int2(x, y));
        };
    };
}

class CameraImpl;

class ScreenBuffer : public RegistrableManaged<float4>,
                     public enable_shared_from_this<ScreenBuffer> {
public:
    using manager_type = ocarina::map<string, SP<ScreenBuffer>>;
    using RegistrableManaged<float4>::RegistrableManaged;
    using Super = RegistrableManaged<float4>;

public:
    ScreenBuffer() = default;
    explicit ScreenBuffer(string key) : Super() {
        name_ = std::move(key);
    }
};


class FrameBuffer : public Node {
public:
    static constexpr auto final_result = "FrameBuffer::final_result_";

protected:
    string cur_view_{final_result};
    /// save two frames of data
    RegistrableBuffer<PixelGeometry> gbuffer_{};

    /// save two frames of data , use for ReSTIR
    RegistrableBuffer<SurfaceData> surfaces_{};
    RegistrableBuffer<HitBSDF> hit_bsdfs_{};
    RegistrableBuffer<float2> motion_vectors_{};

    /// used for editor
    RegistrableManaged<Hit> hit_buffer_;
    ScreenBuffer::manager_type screen_buffers_;
    Shader<void(Buffer<float4>, Buffer<float4>)> gamma_correct_;
    /// Display in full screen on the screen
    Buffer<float4> view_buffer_;

public:
    using Desc = FrameBufferDesc;

public:
    explicit FrameBuffer(const FrameBufferDesc &desc);
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] uint pixel_num() const noexcept;
    [[nodiscard]] uint2 resolution() const noexcept;
    [[nodiscard]] uint pixel_index(uint2 pos) const noexcept;
    [[nodiscard]] uint gbuffer_base() const noexcept { return gbuffer_.index().hv(); }
    [[nodiscard]] uint surface_base() const noexcept { return surfaces_.index().hv(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T prev_gbuffer_index(const T &frame_index) const noexcept { return prev_index(frame_index) + gbuffer_base(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T cur_gbuffer_index(const T &frame_index) const noexcept { return cur_index(frame_index) + gbuffer_base(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T cur_surfaces_index(const T &frame_index) const noexcept { return cur_index(frame_index) + surface_base(); }
    template<typename T>
    requires is_integral_expr_v<T>
    [[nodiscard]] T prev_surfaces_index(const T &frame_index) const noexcept { return prev_index(frame_index) + surface_base(); }

    [[nodiscard]] BufferView<PixelGeometry> prev_gbuffer(uint frame_index) const noexcept;
    [[nodiscard]] BufferView<PixelGeometry> cur_gbuffer(uint frame_index) const noexcept;

    [[nodiscard]] const Buffer<float4> &view_buffer() const noexcept;

    void register_(const SP<ScreenBuffer> &buffer) noexcept;
    void unregister(const SP<ScreenBuffer> &buffer) noexcept;
    void unregister(const string &name) noexcept;
    void init_screen_buffer(const SP<ScreenBuffer> &buffer) noexcept;
    void prepare_screen_buffer(const SP<ScreenBuffer> &buffer) noexcept {
        init_screen_buffer(buffer);
        register_(buffer);
    }

#define VS_MAKE_ATTR_FUNC(buffer_name, count)                             \
    OC_MAKE_MEMBER_GETTER(buffer_name, &)                                 \
    void prepare_##buffer_name() noexcept {                               \
        init_buffer(buffer_name##_, "FrameBuffer::" #buffer_name, count); \
    }

    VS_MAKE_ATTR_FUNC(surfaces, 2)
    VS_MAKE_ATTR_FUNC(gbuffer, 2)
    VS_MAKE_ATTR_FUNC(hit_bsdfs, 1)
    VS_MAKE_ATTR_FUNC(motion_vectors, 1)
    VS_MAKE_ATTR_FUNC(hit_buffer, 1)

#undef VS_MAKE_ATTR_FUNC

    [[nodiscard]] BindlessArray &bindless_array() noexcept;
    [[nodiscard]] virtual CommandList compute_GBuffer(uint frame_index, BufferView<PixelGeometry> gbuffer, BufferView<float2> motion_vectors,
                                                      BufferView<float4> albedo, BufferView<float4> emission) const noexcept = 0;
    [[nodiscard]] virtual CommandList compute_geom(uint frame_index, BufferView<PixelGeometry> gbuffer, BufferView<float2> motion_vectors,
                                                   BufferView<float4> albedo, BufferView<float4> emission) const noexcept = 0;
    [[nodiscard]] virtual CommandList compute_grad(uint frame_index, BufferView<PixelGeometry> gbuffer) const noexcept = 0;
    [[nodiscard]] virtual CommandList compute_hit() const noexcept = 0;
    [[nodiscard]] static Float2 compute_motion_vec(const Camera &camera, const Float2 &p_film, const Float3 &cur_pos,
                                                   const Bool &is_hit) noexcept;
    virtual void compile() noexcept;
    [[nodiscard]] CommandList gamma_correct(BufferView<float4> input,
                                            BufferView<float4> output) const noexcept;
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

    template<typename T>
    void init_buffer(RegistrableManaged<T> &buffer, const string &desc, uint count = 1) noexcept {
        uint element_num = count * pixel_num();
        buffer.reset_all(device(), element_num, desc);
        vector<T> vec{};
        vec.assign(element_num, T{});
        buffer.set_bindless_array(bindless_array());
        buffer.register_self();
        for (int i = 1; i < count; ++i) {
            buffer.register_view(pixel_num() * i, pixel_num());
        }
    }
};

}// namespace vision
