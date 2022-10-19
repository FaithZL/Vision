//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "base/sensor.h"
#include "descriptions/scene_desc.h"

namespace vision {

using namespace ocarina;

class Context;

class Scene {
private:
    vision::Context *_context{nullptr};
    Camera *_camera{nullptr};
public:
    explicit Scene(vision::Context *ctx);
    void init(const SceneDesc& scene_desc);
    void prepare(RenderPipeline *rp) noexcept;
};

}// namespace vision