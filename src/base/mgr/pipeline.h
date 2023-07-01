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

#define VS_MAKE_GETTER(member)                                                 \
    [[nodiscard]] const ImageIO &member() const noexcept { return _##member; } \
    [[nodiscard]] ImageIO &member() noexcept { return _##member; }

class Pipeline : public Node {
public:
    using Desc = PipelineDesc;

protected:
    Device *_device{};
    Scene _scene{};
    Geometry _geometry{this};
    ResourceArray _resource_array{};
    mutable Stream _stream;
    uint _frame_index{};
    double _total_time{};
    RegistrableManaged<float4> _final_picture;
    Postprocessor _postprocessor{this};

protected:
    [[nodiscard]] Integrator *integrator() noexcept { return scene().integrator(); }
    [[nodiscard]] const Integrator *integrator() const noexcept { return scene().integrator(); }

public:
    explicit Pipeline(Device *device);
    explicit Pipeline(const PipelineDesc &desc);
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] Scene &scene() noexcept { return _scene; }
    [[nodiscard]] const Scene &scene() const noexcept { return _scene; }

    /// virtual function start
    virtual void init_scene(const SceneDesc &scene_desc) = 0;
    virtual void init_postprocessor(const SceneDesc &scene_desc) = 0;
    [[nodiscard]] virtual const Buffer<float4> &view_buffer();
    virtual void preprocess() noexcept {}
    virtual void change_resolution(uint2 res) noexcept;
    virtual void invalidate() noexcept {
        _frame_index = 0;
        _total_time = 0;
    }
    virtual void prepare_geometry() noexcept;
    virtual void prepare_render_graph() noexcept {}
    virtual void compile_shaders() noexcept = 0;
    virtual void display(double dt) noexcept = 0;
    virtual void render(double dt) noexcept = 0;
    virtual void upload_data() noexcept { _scene.upload_data(); }
    [[nodiscard]] virtual float4 *final_picture(bool denoise) noexcept;
    [[nodiscard]] virtual uint2 resolution() const noexcept { return _scene.camera()->resolution(); }
    [[nodiscard]] uint pixel_num() const noexcept { return resolution().x * resolution().y; }
    /// virtual function end

    template<typename T>
    requires is_buffer_or_view_v<T>
    [[nodiscard]] handle_ty register_buffer(T &&buffer) noexcept {
        return _resource_array.emplace(OC_FORWARD(buffer));
    }
    handle_ty register_texture(const Texture &texture) noexcept {
        return _resource_array.emplace(texture);
    }
    template<typename T>
    requires is_buffer_or_view_v<T>
    void set_buffer(handle_ty index, T &&buffer) noexcept {
        _resource_array.set_buffer(index, OC_FORWARD(buffer));
    }
    void set_texture(handle_ty index, const Texture &texture) noexcept {
        _resource_array.set_texture(index, texture);
    }
    void deregister_buffer(handle_ty index) noexcept;
    void deregister_texture(handle_ty index) noexcept;
    [[nodiscard]] ImagePool &image_pool() noexcept { return Global::instance().image_pool(); }
    [[nodiscard]] ResourceArray &resource_array() noexcept { return _resource_array; }
    [[nodiscard]] const ResourceArray &resource_array() const noexcept { return _resource_array; }
    void prepare_resource_array() noexcept;
    [[nodiscard]] Geometry &geometry() noexcept { return _geometry; }
    [[nodiscard]] const Geometry &geometry() const noexcept { return _geometry; }
    [[nodiscard]] ImageWrapper &obtain_image(const ShaderNodeDesc &desc) noexcept {
        return image_pool().obtain_image(desc);
    }
    [[nodiscard]] uint frame_index() const noexcept { return _frame_index; }
    [[nodiscard]] Stream &stream() const noexcept { return _stream; }

    /// for dsl
    [[nodiscard]] OCHit trace_closest(const OCRay &ray) const noexcept;
    [[nodiscard]] Bool trace_any(const OCRay &ray) const noexcept;

    [[nodiscard]] Interaction compute_surface_interaction(const OCHit &hit, OCRay &ray) const noexcept;
    [[nodiscard]] LightEvalContext compute_light_eval_context(const Uint &inst_id,
                                                              const Uint &prim_id,
                                                              const Float2 &bary) const noexcept;

    template<typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] ResourceArrayTexture tex(Index &&index) const noexcept {
        Uint i = OC_FORWARD(index);
#ifndef NDEBUG
        $if(i >= _resource_array.texture_num()) {
            string tb = traceback_string();
            string fmt = "out of texture num: texture index is {}, texture size is {}, traceback is " + tb;
            Printer::instance().warn(fmt, i, _resource_array.texture_num());
            i = 0;
        };
#endif
        return _resource_array.tex(i);
    }

    template<typename T, typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] ResourceArrayBuffer<T> buffer(Index &&index) const noexcept {
        return _resource_array.buffer<T>(OC_FORWARD(index));
    }

    template<typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] ResourceArrayByteBuffer byte_buffer(Index &&index) const noexcept {
        return _resource_array.byte_buffer(OC_FORWARD(index));
    }
};

#undef VS_MAKE_GETTER

}// namespace vision