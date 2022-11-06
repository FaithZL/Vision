//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/scene.h"
#include "base/geometry.h"
#include "rhi/window.h"
#include "util/image_io.h"

namespace vision {
using namespace ocarina;

class RenderPipeline {
private:
    Device *_device;
    vision::Context *_context;
    Scene _scene;
    Geometry _geometry{_device};
    ImageIO _render_image;
    Stream _stream;
    uint _frame_index{};
    double _total_time{};

public:
    RenderPipeline(Device *device, vision::Context *context);
    void init_scene(const SceneDesc &scene_desc) { _scene.init(scene_desc); }
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] Scene &scene() noexcept { return _scene; }
    [[nodiscard]] Geometry &geometry() noexcept { return _geometry; }
    [[nodiscard]] const Geometry &geometry() const noexcept { return _geometry; }
    [[nodiscard]] vision::Context &context() noexcept { return *_context; }
    void update() noexcept { _frame_index = 0; _total_time = 0; }
    [[nodiscard]] uint frame_index() const noexcept { return _frame_index; }
    void prepare() noexcept;
    [[nodiscard]] Stream &stream() noexcept { return _stream; }
    void prepare_device_data() noexcept;
    void compile_shaders() noexcept;
    [[nodiscard]] uint2 resolution() const noexcept { return _scene.camera()->resolution(); }
    void download_result();
    [[nodiscard]] const float4 *buffer() const { return _render_image.pixel_ptr<float4>(); }
    void upload_data() noexcept { _scene.upload_data(); }
    void render(double dt) noexcept;

    // for dsl
    [[nodiscard]] OCHit trace_closest(const OCRay &ray) const noexcept;
    [[nodiscard]] Bool trace_any(const OCRay &ray) const noexcept;
    [[nodiscard]] SurfaceInteraction compute_surface_interaction(const OCHit &hit) const noexcept;
    [[nodiscard]] LightEvalContext compute_light_eval_context(const Uint &inst_id,
                                                              const Uint &prim_id,
                                                              const Float2 &bary) const noexcept;
    template<typename T>
    void dispatch(const Uint &id, const vector<T *> &lst, const std::function<void(const T *)> &func) {
        if (lst.empty()) [[unlikely]] { OC_ERROR("lst is empty"); }
        $switch(id) {
            for (int i = 0; i < lst.size(); ++i) {
                $case(i) {
                    func(lst[i]);
                    $break;
                };
            }
            $default {
                unreachable();
                $break;
            };
        };
    }
};

}// namespace vision