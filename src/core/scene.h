//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "base/filter.h"
#include "descriptions/scene_desc.h"

namespace vision {

using namespace ocarina;

class Context;

class Scene {
private:
    vision::Context *_context{nullptr};

public:
    explicit Scene(vision::Context *ctx);
    void prepare(SceneDesc scene_desc);
};

}// namespace vision