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

class RenderPipeline {
private:
    Device *_device;
    vision::Context *_context;
    Scene _scene;
    Geometry _geometry{_device};
    mutable Stream _stream;
    uint _frame_index{};
    ImagePool _image_pool{_device};
    double _total_time{};
    using image_clear_signature = void(RHITexture);
    ocarina::Kernel<image_clear_signature> _kernel;
    ocarina::Shader<image_clear_signature> _shader;

public:
    RenderPipeline(Device *device, vision::Context *context);
    void clear_image(RHITexture &image) const noexcept;
    void init_scene(const SceneDesc &scene_desc) { _scene.init(scene_desc); }
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] Scene &scene() noexcept { return _scene; }
    void change_resolution(uint2 res) noexcept;
    [[nodiscard]] Geometry &geometry() noexcept { return _geometry; }
    [[nodiscard]] const Geometry &geometry() const noexcept { return _geometry; }
    [[nodiscard]] ImageWrapper &obtain_image(const TextureDesc &desc) noexcept {
        return _image_pool.obtain_image(desc);
    }
    [[nodiscard]] vision::Context &context() noexcept { return *_context; }
    void update() noexcept { _frame_index = 0; _total_time = 0; }
    [[nodiscard]] uint frame_index() const noexcept { return _frame_index; }
    void prepare() noexcept;
    [[nodiscard]] Stream &stream() noexcept { return _stream; }
    void prepare_device_data() noexcept;
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
    // for dsl
    [[nodiscard]] OCHit trace_closest(const OCRay &ray) const noexcept;
    [[nodiscard]] Bool trace_any(const OCRay &ray) const noexcept;
    [[nodiscard]] Interaction compute_surface_interaction(const OCHit &hit, OCRay &ray) const noexcept;
    [[nodiscard]] LightEvalContext compute_light_eval_context(const Uint &inst_id,
                                                              const Uint &prim_id,
                                                              const Float2 &bary) const noexcept;
};

}// namespace vision