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

namespace vision {
using namespace ocarina;
class Spectrum;
class RenderPipeline {
private:
    Device *_device;
    vision::Context *_context;
    Scene _scene;
    Geometry _geometry{this};
    ResourceArray _resource_array{};
    mutable Stream _stream;
    uint _frame_index{};
    ImagePool _image_pool{this};
    double _total_time{};

public:
    RenderPipeline(Device *device, vision::Context *context);
    void init_scene(const SceneDesc &scene_desc) { _scene.init(scene_desc); }
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] Scene &scene() noexcept { return _scene; }
    template<typename T>
    requires is_buffer_or_view_v<T>
    [[nodiscard]] handle_ty register_buffer(T &&buffer) noexcept {
        return _resource_array.emplace(OC_FORWARD(buffer));
    }
    handle_ty register_texture(const RHITexture &texture) noexcept {
        return _resource_array.emplace(texture);
    }
    void deregister_buffer(handle_ty index) noexcept;
    void deregister_texture(handle_ty index) noexcept;
    [[nodiscard]] ResourceArray &resource_array() noexcept { return _resource_array; }
    void prepare_resource_array() noexcept;
    [[nodiscard]] Spectrum &spectrum() noexcept;
    [[nodiscard]] const Spectrum &spectrum() const noexcept;
    void change_resolution(uint2 res) noexcept;
    [[nodiscard]] Geometry &geometry() noexcept { return _geometry; }
    [[nodiscard]] const Geometry &geometry() const noexcept { return _geometry; }
    [[nodiscard]] ImageWrapper &obtain_image(const ShaderNodeDesc &desc) noexcept {
        return _image_pool.obtain_image(desc);
    }
    [[nodiscard]] vision::Context &context() noexcept { return *_context; }
    void update() noexcept {
        _frame_index = 0;
        _total_time = 0;
    }
    [[nodiscard]] uint frame_index() const noexcept { return _frame_index; }
    void prepare() noexcept;
    [[nodiscard]] Stream &stream() const noexcept { return _stream; }
    void prepare_geometry() noexcept;
    void compile_shaders() noexcept;
    [[nodiscard]] uint2 resolution() const noexcept { return _scene.camera()->resolution(); }
    void download_result(void *ptr) noexcept {
        _scene.film()->copy_to(ptr);
    }
    void upload_data() noexcept { _scene.upload_data(); }
    void render(double dt) noexcept;
    void render_to_image(double dt, void *ptr) {
        render(dt);
        download_result(ptr);
    }
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
        return _resource_array.tex(OC_FORWARD(index));
    }

    template<typename T, typename Index>
    requires is_integral_expr_v<Index>
    [[nodiscard]] ResourceArrayBuffer<T> buffer(Index &&index) const noexcept {
        return _resource_array.buffer<T>(OC_FORWARD(index));
    }
};

}// namespace vision