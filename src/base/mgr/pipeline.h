//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "scene.h"
#include "geometry.h"
#include "rhi/window.h"
#include "util/image_io.h"
#include "image_pool.h"
#include "postprocessor.h"

namespace vision {
using namespace ocarina;
class Spectrum;

class Pipeline : public Node {
public:
    using Desc = PipelineDesc;

protected:
    Device *_device{};
    Scene _scene{};
    Geometry _geometry{this};
    BindlessArray _bindless_array{};
    mutable Stream _stream;
    bool _show_fps{true};
    RegistrableManaged<float4> _final_picture;
    Postprocessor _postprocessor{this};

protected:
    [[nodiscard]] Integrator *integrator() noexcept { return scene().integrator(); }
    [[nodiscard]] const Integrator *integrator() const noexcept { return scene().integrator(); }

public:
    OutputDesc output_desc;

public:
    explicit Pipeline(const PipelineDesc &desc);
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] Scene &scene() noexcept { return _scene; }
    [[nodiscard]] const Scene &scene() const noexcept { return _scene; }

    /// virtual function start
    virtual void init_scene(const SceneDesc &scene_desc) = 0;
    virtual void init_postprocessor(const DenoiserDesc &desc) = 0;
    [[nodiscard]] virtual const Buffer<float4> &view_buffer();
    virtual void preprocess() noexcept {}
    virtual void change_resolution(uint2 res) noexcept;
    virtual void invalidate() noexcept { integrator()->invalidation(); }
    virtual void clear_geometry() noexcept;
    virtual void prepare_geometry() noexcept;
    virtual void update_geometry() noexcept;
    virtual void prepare_render_graph() noexcept {}
    virtual void compile() noexcept = 0;
    virtual void display(double dt) noexcept;
    virtual void render(double dt) noexcept = 0;
    virtual void before_render() noexcept;
    virtual void after_render() noexcept;
    virtual void upload_data() noexcept { _scene.upload_data(); }
    [[nodiscard]] virtual float4 *final_picture(const OutputDesc &desc) noexcept;
    [[nodiscard]] virtual uint2 resolution() const noexcept { return _scene.camera()->resolution(); }
    [[nodiscard]] uint pixel_num() const noexcept { return resolution().x * resolution().y; }
    /// virtual function end

    [[nodiscard]] virtual uint frame_index() const noexcept { return integrator()->frame_index(); }
    [[nodiscard]] double render_time() const noexcept { return integrator()->render_time(); }
    static void flip_debugger() noexcept { Env::debugger().filp_enabled(); }
    void filp_show_fps() noexcept { _show_fps = !_show_fps; }
    template<typename T>
    requires is_buffer_or_view_v<T>
    [[nodiscard]] handle_ty register_buffer(T &&buffer) noexcept {
        return _bindless_array.emplace(OC_FORWARD(buffer));
    }
    handle_ty register_texture(const Texture &texture) noexcept {
        return _bindless_array.emplace(texture);
    }
    template<typename T>
    requires is_buffer_or_view_v<T>
    void set_buffer(handle_ty index, T &&buffer) noexcept {
        _bindless_array.set_buffer(index, OC_FORWARD(buffer));
    }
    void set_texture(handle_ty index, const Texture &texture) noexcept {
        _bindless_array.set_texture(index, texture);
    }
    void deregister_buffer(handle_ty index) noexcept;
    void deregister_texture(handle_ty index) noexcept;
    [[nodiscard]] ImagePool &image_pool() noexcept { return Global::instance().image_pool(); }
    [[nodiscard]] BindlessArray &bindless_array() noexcept { return _bindless_array; }
    [[nodiscard]] const BindlessArray &bindless_array() const noexcept { return _bindless_array; }
    void upload_bindless_array() noexcept;
    [[nodiscard]] Geometry &geometry() noexcept { return _geometry; }
    [[nodiscard]] const Geometry &geometry() const noexcept { return _geometry; }
    [[nodiscard]] ImageWrapper &obtain_image(const ShaderNodeDesc &desc) noexcept {
        return image_pool().obtain_image(desc);
    }
    [[nodiscard]] Stream &stream() const noexcept { return _stream; }

    /// for dsl
    template<typename... Args>
    [[nodiscard]] OCHit trace_closest(Args &&...args) const noexcept {
        return geometry().trace_closest(OC_FORWARD(args)...);
    }
    template<typename... Args>
    [[nodiscard]] Bool trace_any(Args &&...args) const noexcept {
        return geometry().trace_any(OC_FORWARD(args)...);
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

    template<typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] BindlessArrayTexture tex(Index &&index) const noexcept {
        return _bindless_array.tex(OC_FORWARD(index));
    }

    template<typename T, typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] BindlessArrayBuffer<T> buffer(Index &&index) const noexcept {
        return _bindless_array.buffer<T>(OC_FORWARD(index));
    }

    template<typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] BindlessArrayByteBuffer byte_buffer(Index &&index) const noexcept {
        return _bindless_array.byte_buffer(OC_FORWARD(index));
    }
};

}// namespace vision