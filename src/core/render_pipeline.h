//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "scene.h"
#include "rhi/window.h"

namespace vision {
using namespace ocarina;
class RenderPipeline {
private:
    Device *_device;
    vision::Context *_context;
    Scene _scene;

public:
    RenderPipeline(Device *device, vision::Context *context);
    void init_scene(const SceneDesc &scene_desc) { _scene.init(scene_desc); }
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] vision::Context &context() noexcept { return *_context; }
    void prepare() noexcept { _scene.prepare(this); }
};

}// namespace vision