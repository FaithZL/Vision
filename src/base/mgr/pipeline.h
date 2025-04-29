//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "scene.h"
#include "geometry.h"
#include "util/image.h"
#include "image_pool.h"
#include "base/frame_buffer.h"
#include "postprocessor.h"
#include "UI/GUI.h"

namespace vision {
using namespace ocarina;
class Spectrum;

class Pipeline : public Node, public Observer {
public:
    using Desc = PipelineDesc;

protected:
    Device *device_{};
    Scene scene_{};
    Geometry geometry_{this};
    BindlessArray bindless_array_{};
    mutable Stream stream_;
    bool show_fps_{true};
    RegistrableManaged<float4> final_picture_;
    Postprocessor postprocessor_{this};
    SP<FrameBuffer> frame_buffer_{nullptr};
    bool show_scene_data_{true};
    bool show_framebuffer_data_{true};
    bool show_detail_{true};
    bool show_stats_{true};
    bool show_hotfix_{true};
    bool show_output_{false};
    bool need_save_{false};

    /// node for show detail
    mutable GUI *cur_node_{nullptr};
    mutable vector<UP<ShaderBase>> shaders_;

protected:
    [[nodiscard]] auto &integrator() noexcept { return scene().integrator(); }
    [[nodiscard]] auto &integrator() const noexcept { return scene().integrator(); }

public:
    OutputDesc output_desc;

public:
    explicit Pipeline(const PipelineDesc &desc);
    void init() noexcept;
    [[nodiscard]] const Device &device() const noexcept { return *device_; }
    [[nodiscard]] Device &device() noexcept { return *device_; }
    [[nodiscard]] Scene &scene() noexcept { return scene_; }
    [[nodiscard]] const Scene &scene() const noexcept { return scene_; }
    [[nodiscard]] auto frame_buffer() const noexcept { return frame_buffer_.get(); }
    [[nodiscard]] auto frame_buffer() noexcept { return frame_buffer_.get(); }
    void on_touch(uint2 pos) noexcept;
    [[nodiscard]] bool has_changed() noexcept override;
    void reset_status() noexcept override;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_detail(Widgets *widgets) noexcept;
    void render_stats(Widgets *widgets) noexcept;
    void render_hotfix(Widgets *widgets) noexcept;
    void render_output(Widgets *widgets) noexcept;
    void save_result() noexcept;
    void check_and_save() noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(cur_node, )

    /// virtual function start
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    virtual void init_scene(const SceneDesc &scene_desc) = 0;
    virtual void init_postprocessor(const DenoiserDesc &desc) = 0;
    [[nodiscard]] virtual const Buffer<float4> &view_buffer();
    virtual void preprocess() noexcept {}
    void prepare() noexcept override;
    virtual void change_resolution(uint2 res) noexcept;
    virtual void invalidate() noexcept { integrator()->invalidation(); }
    virtual void clear_geometry() noexcept;
    virtual void prepare_geometry() noexcept;
    virtual void update_geometry() noexcept;
    virtual void prepare_render_graph() noexcept {}
    virtual void compile() noexcept {
        frame_buffer()->compile();
    }
    virtual void display(double dt) noexcept;
    virtual void render(double dt) noexcept = 0;
    virtual void commit_command() noexcept;
    virtual void before_render() noexcept;
    virtual void after_render() noexcept;
    virtual void upload_data() noexcept { scene_.upload_data(); }
    [[nodiscard]] virtual float4 *final_picture(const OutputDesc &desc) noexcept;
    [[nodiscard]] virtual uint2 resolution() const noexcept { return scene_.sensor()->resolution(); }
    [[nodiscard]] uint pixel_num() const noexcept { return resolution().x * resolution().y; }
    /// virtual function end

    template<typename T>
    [[nodiscard]] CommandList reset_buffer(BufferView<T> buffer, T elm = T{},
                                           string desc = "clear_buffer") const noexcept {
        static Kernel kernel = [&](BufferVar<T> buffer_var, Var<T> value) {
            buffer_var.write(dispatch_id(), value);
        };
        using shader_t = decltype(device().compile(kernel, desc));
        static shader_t* shader = [&]{
            UP<shader_t> uptr = make_unique<shader_t >(device().compile(kernel, desc));
            auto ret = static_cast<shader_t *>(uptr.get());
            shaders_.push_back(std::move(uptr));
            return ret;
        }();
        CommandList ret;
        ret << (*shader)(buffer, elm).dispatch(buffer.size());
        return ret;
    }

    [[nodiscard]] virtual uint frame_index() const noexcept { return integrator()->frame_index(); }
    [[nodiscard]] double render_time() const noexcept { return integrator()->render_time(); }
    [[nodiscard]] double cur_render_time() const noexcept { return integrator()->cur_render_time(); }
    static void flip_debugger() noexcept { Env::debugger().filp_enabled(); }
    void filp_show_fps() noexcept { show_fps_ = !show_fps_; }
    template<typename T>
    requires is_buffer_or_view_v<T>
    [[nodiscard]] handle_ty register_buffer(T &&buffer) noexcept {
        return bindless_array_.emplace(OC_FORWARD(buffer));
    }
    handle_ty register_texture(const Texture &texture) noexcept {
        return bindless_array_.emplace(texture);
    }
    template<typename T>
    requires is_buffer_or_view_v<T>
    void set_buffer(handle_ty index, T &&buffer) noexcept {
        bindless_array_.set_buffer(index, OC_FORWARD(buffer));
    }
    void set_texture(handle_ty index, const Texture &texture) noexcept {
        bindless_array_.set_texture(index, texture);
    }
    void deregister_buffer(handle_ty index) noexcept;
    void deregister_texture(handle_ty index) noexcept;
    [[nodiscard]] ImagePool &image_pool() noexcept { return Global::instance().image_pool(); }
    [[nodiscard]] BindlessArray &bindless_array() noexcept { return bindless_array_; }
    [[nodiscard]] const BindlessArray &bindless_array() const noexcept { return bindless_array_; }
    void upload_bindless_array() noexcept;
    [[nodiscard]] Geometry &geometry() noexcept { return geometry_; }
    [[nodiscard]] const Geometry &geometry() const noexcept { return geometry_; }
    [[nodiscard]] Stream &stream() const noexcept { return stream_; }

    template<typename... Args>
    void denoise(Args &&...args) const noexcept {
        postprocessor_.denoise(OC_FORWARD(args)...);
    }

    /// for dsl
    template<typename... Args>
    [[nodiscard]] TriangleHitVar trace_closest(Args &&...args) const noexcept {
        return geometry().trace_closest(OC_FORWARD(args)...);
    }
    template<typename... Args>
    [[nodiscard]] Bool trace_occlusion(Args &&...args) const noexcept {
        return geometry().trace_occlusion(OC_FORWARD(args)...);
    }
    template<typename... Args>
    [[nodiscard]] auto visibility(Args &&...args) const noexcept {
        return geometry().visibility(OC_FORWARD(args)...);
    }
    template<typename... Args>
    [[nodiscard]] Interaction compute_surface_interaction(Args &&...args) const noexcept {
        return geometry().compute_surface_interaction(OC_FORWARD(args)...);
    }
    template<typename... Args>
    [[nodiscard]] LightEvalContext compute_light_eval_context(Args &&...args) const noexcept {
        return geometry().compute_light_eval_context(OC_FORWARD(args)...);
    }

    template<typename T>
    [[nodiscard]] BufferView<T> buffer_view(uint index) const noexcept {
        return bindless_array_.buffer_view<T>(index);
    }

    template<typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] BindlessArrayTexture tex_var(Index &&index) const noexcept {
        return bindless_array_.tex_var(OC_FORWARD(index));
    }

    template<typename T, typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] BindlessArrayBuffer<T> buffer_var(Index &&index) const noexcept {
        return bindless_array_.buffer_var<T>(OC_FORWARD(index));
    }

    template<typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] BindlessArrayByteBuffer byte_buffer_var(Index &&index) const noexcept {
        return bindless_array_.byte_buffer_var(OC_FORWARD(index));
    }

    template<typename Elm, typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] SOAView<Elm, BindlessArrayByteBuffer> soa_view(Index &&index) noexcept {
        return bindless_array_.soa_view<Elm>(OC_FORWARD(index));
    }
};

}// namespace vision