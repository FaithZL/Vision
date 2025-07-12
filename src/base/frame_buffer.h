//
// Created by Zero on 2024/2/17.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "sensor/sensor.h"
#include "scattering/interaction.h"
#include "visualizer.h"

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

class Sensor;

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
    [[nodiscard]] const Super &super() const { return *this; }
    [[nodiscard]] Super &super() { return *this; }
    void update_resolution(uint2 res, Device &device) noexcept;
};

class FrameBuffer : public Node, public Observer {
public:
    static constexpr auto final_result = "FrameBuffer::final_result_";

protected:
    using gbuffer_signature = void(uint, Buffer<PixelGeometry>, Buffer<float2>, Buffer<float4>, Buffer<float4>, Buffer<float4>);
    Shader<gbuffer_signature> compute_geom_;

    using grad_signature = void(uint, Buffer<PixelGeometry>);
    Shader<grad_signature> compute_grad_;

    Shader<void(Buffer<TriangleHit>, uint)> compute_hit_;

protected:
    string cur_view_{final_result};
    ScreenBuffer::manager_type screen_buffers_;
    Shader<void(Buffer<float4>, Buffer<float4>)> gamma_correct_;

    SP<Visualizer> visualizer_{make_shared<Visualizer>()};

    vector<float4> window_buffer_;

#define VS_MAKE_BUFFER(Type, buffer_name, count)                           \
protected:                                                                 \
    Type buffer_name##_;                                                   \
                                                                           \
public:                                                                    \
    OC_MAKE_MEMBER_GETTER(buffer_name, &)                                  \
    void prepare_##buffer_name() noexcept {                                \
        init_buffer(buffer_name##_, "FrameBuffer::" #buffer_name, count);  \
    }                                                                      \
    void reset_##buffer_name() noexcept {                                  \
        reset_buffer(buffer_name##_, "FrameBuffer::" #buffer_name, count); \
    }                                                                      \
    [[nodiscard]] uint buffer_name##_base() const noexcept {               \
        return buffer_name##_.index().hv();                                \
    }

#define VS_MAKE_DOUBLE_BUFFER(Type, buffer_name)                                                                              \
    VS_MAKE_BUFFER(Type, buffer_name, 2)                                                                                      \
    template<typename T>                                                                                                      \
    requires is_integral_expr_v<T>                                                                                            \
    [[nodiscard]] T prev_##buffer_name##_index(const T &frame_index) const noexcept {                                         \
        return prev_index(frame_index) + buffer_name##_base();                                                                \
    }                                                                                                                         \
    template<typename T>                                                                                                      \
    requires is_integral_expr_v<T>                                                                                            \
    [[nodiscard]] T cur_##buffer_name##_index(const T &frame_index) const noexcept {                                          \
        return cur_index(frame_index) + buffer_name##_base();                                                                 \
    }                                                                                                                         \
    [[nodiscard]] auto prev_##buffer_name##_view(uint frame_index) const noexcept {                                           \
        return bindless_array().buffer_view<decltype(buffer_name##_)::element_type>(prev_##buffer_name##_index(frame_index)); \
    }                                                                                                                         \
    [[nodiscard]] auto cur_##buffer_name##_view(uint frame_index) const noexcept {                                            \
        return bindless_array().buffer_view<decltype(buffer_name##_)::element_type>(cur_##buffer_name##_index(frame_index));  \
    }                                                                                                                         \
    [[nodiscard]] auto prev_##buffer_name##_var(const Uint &frame_index) const noexcept {                                     \
        return bindless_array().buffer_var<decltype(buffer_name##_)::element_type>(prev_##buffer_name##_index(frame_index));  \
    }                                                                                                                         \
    [[nodiscard]] auto cur_##buffer_name##_var(const Uint &frame_index) const noexcept {                                      \
        return bindless_array().buffer_var<decltype(buffer_name##_)::element_type>(cur_##buffer_name##_index(frame_index));   \
    }                                                                                                                         \
    [[nodiscard]] auto prev_##buffer_name##_byte_view(uint frame_index) const noexcept {                                      \
        return ByteBufferView(prev_##buffer_name##_view(frame_index));                                                        \
    }                                                                                                                         \
    [[nodiscard]] auto cur_##buffer_name##_byte_view(uint frame_index) const noexcept {                                       \
        return ByteBufferView(cur_##buffer_name##_view(frame_index));                                                         \
    }

    /// save two frames of data , use for ReSTIR
    VS_MAKE_DOUBLE_BUFFER(RegistrableBuffer<SurfaceData>, surfaces)
    VS_MAKE_DOUBLE_BUFFER(RegistrableBuffer<SurfaceExtend>, surface_exts)
    VS_MAKE_BUFFER(RegistrableBuffer<float2>, motion_vectors, 1)
    VS_MAKE_BUFFER(RegistrableBuffer<HitBSDF>, hit_bsdfs, 1)
    VS_MAKE_BUFFER(RegistrableBuffer<float4>, emission, 1)
    VS_MAKE_BUFFER(RegistrableManaged<float4>, albedo, 1)
    VS_MAKE_BUFFER(RegistrableManaged<float4>, normal, 1)

    VS_MAKE_DOUBLE_BUFFER(RegistrableBuffer<PixelGeometry>, gbuffer)

    /// used for editor
    VS_MAKE_BUFFER(RegistrableManaged<TriangleHit>, hit_buffer, 1)

    /// Display in full screen on the screen
    VS_MAKE_BUFFER(RegistrableBuffer<float4>, view_buffer, 1)

#undef VS_MAKE_DOUBLE_BUFFER

#undef VS_MAKE_BUFFER

public:
    using Desc = FrameBufferDesc;

public:
    FrameBuffer() = default;
    explicit FrameBuffer(const FrameBufferDesc &desc);
    VS_HOTFIX_MAKE_RESTORE(Node, cur_view_, gbuffer_, surfaces_, surface_exts_, hit_bsdfs_,
                           motion_vectors_, hit_buffer_, screen_buffers_, gamma_correct_,
                           view_buffer_, visualizer_, window_buffer_,
                           compute_geom_, compute_grad_, compute_hit_)
    void prepare() noexcept override;
    void update_runtime_object(const IObjectConstructor *constructor) noexcept override;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    OC_MAKE_MEMBER_GETTER(visualizer, &)
    OC_MAKE_MEMBER_GETTER(window_buffer, &)
    void fill_window_buffer(const Buffer<float4> &input) noexcept;
    void resize(uint2 res) noexcept;
    virtual void update_resolution(uint2 res) noexcept;
    [[nodiscard]] uint pixel_num() const noexcept;
    [[nodiscard]] uint2 resolution() const noexcept;
    [[nodiscard]] uint pixel_index(uint2 pos) const noexcept;
    [[nodiscard]] BindlessArray &bindless_array() const noexcept;

    [[nodiscard]] const Buffer<float4> &cur_screen_buffer() const noexcept;

    void register_(const SP<ScreenBuffer> &buffer) noexcept;
    void unregister(const SP<ScreenBuffer> &buffer) noexcept;
    void unregister(const string &name) noexcept;
    void init_screen_buffer(const SP<ScreenBuffer> &buffer) noexcept;
    void prepare_screen_buffer(const SP<ScreenBuffer> &buffer) noexcept {
        init_screen_buffer(buffer);
        register_(buffer);
    }

    [[nodiscard]] BindlessArray &bindless_array() noexcept;
    void after_render() noexcept;
    [[nodiscard]] static Float2 compute_motion_vec(const TSensor &camera, const Float2 &p_film, const Float3 &cur_pos,
                                                   const Bool &is_hit) noexcept;
    [[nodiscard]] Float3 compute_motion_vector(const TSensor &camera, const Float2 &p_film, const Uint &frame_index) const noexcept;
    [[nodiscard]] Float3 compute_motion_vector(const TSensor &camera, const Float3 &cur_pos, const Float3 &pre_pos) const noexcept;
    [[nodiscard]] static Uint checkerboard_value(const Uint2 &coord) noexcept;
    [[nodiscard]] static Uint checkerboard_value(const Uint2 &coord, const Uint &frame_index) noexcept;
    virtual void compile() noexcept;
    void compile_compute_geom() noexcept;
    void compile_compute_grad() noexcept;
    void compile_compute_hit() noexcept;
    void compile_gamma() noexcept;
    void compute_gradient(PixelGeometryVar &center_data,
                          const BufferVar<PixelGeometry> &gbuffer) const noexcept;
    [[nodiscard]] CommandList gamma_correct(BufferView<float4> input,
                                            BufferView<float4> output) const noexcept;
    [[nodiscard]] CommandList gamma_correct() const noexcept;
    [[nodiscard]] virtual CommandList compute_GBuffer(uint frame_index, BufferView<PixelGeometry> gbuffer,
                                                      BufferView<float2> motion_vectors, BufferView<float4> albedo,
                                                      BufferView<float4> emission, BufferView<float4> normal) const noexcept;
    [[nodiscard]] virtual CommandList compute_geom(uint frame_index, BufferView<PixelGeometry> gbuffer,
                                                   BufferView<float2> motion_vectors, BufferView<float4> albedo,
                                                   BufferView<float4> emission, BufferView<float4> normal) const noexcept;
    [[nodiscard]] virtual CommandList compute_grad(uint frame_index, BufferView<PixelGeometry> gbuffer) const noexcept;
    [[nodiscard]] virtual CommandList compute_hit(uint frame_index) const noexcept;

    template<typename T>
    void init_buffer_impl(RegistrableBuffer<T> &buffer, bool has_register, const string &desc, uint count = 1) noexcept {
        uint element_num = count * pixel_num();
        buffer.super() = device().create_buffer<T>(element_num, desc);
        vector<T> vec{};
        vec.assign(element_num, T{});
        buffer.upload_immediately(vec.data());
        buffer.set_bindless_array(bindless_array());
        buffer.register_self();
        for (int i = 1; i < count; ++i) {
            if (has_register) {
                buffer.register_view_index(i, pixel_num() * i, pixel_num());
            } else {
                buffer.register_view(pixel_num() * i, pixel_num());
            }
        }
    }

    template<typename T>
    void init_buffer(RegistrableBuffer<T> &buffer, const string &desc, uint count = 1) noexcept {
        init_buffer_impl(buffer, false, desc, count);
    }

    template<typename T>
    void reset_buffer(RegistrableBuffer<T> &buffer, const string &desc, uint count = 1) noexcept {
        if (buffer.size() == 0) {
            return;
        }
        init_buffer_impl(buffer, true, desc, count);
    }

    template<typename T>
    void init_buffer_impl(RegistrableManaged<T> &buffer, bool has_register, const string &desc, uint count = 1) noexcept {
        uint element_num = count * pixel_num();
        buffer.reset_all(device(), element_num, desc);
        buffer.host_buffer().assign(element_num, T{});
        buffer.upload_immediately();
        buffer.set_bindless_array(bindless_array());
        buffer.register_self();
        for (int i = 1; i < count; ++i) {
            if (has_register) {
                buffer.register_view_index(i, pixel_num() * i, pixel_num());
            } else {
                buffer.register_view(pixel_num() * i, pixel_num());
            }
        }
    }

    template<typename T>
    void init_buffer(RegistrableManaged<T> &buffer, const string &desc, uint count = 1) noexcept {
        init_buffer_impl(buffer, false, desc, count);
    }

    template<typename T>
    void reset_buffer(RegistrableManaged<T> &buffer, const string &desc, uint count = 1) noexcept {
        if (buffer.device_buffer().size() == 0) {
            return;
        }
        init_buffer_impl(buffer, true, desc, count);
    }
};

}// namespace vision
