//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "interaction.h"
#include "scene_node.h"

namespace vision {

// LightType Definition
enum class LightType : uint8_t {
    DeltaPosition,
    DeltaDirection,
    Area,
    Infinite
};

class Light : public SceneNode {
protected:
    const LightType _type{LightType::Area};

public:
    explicit Light(LightType light_type) : _type(light_type) {}
};
}// namespace vision