//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "interaction.h"
#include "node.h"
#include "sample.h"

namespace vision {

// LightType Definition
enum class LightType : uint8_t {
    DeltaPosition,
    DeltaDirection,
    Area,
    Infinite
};

class Light : public Node {
public:
    using Desc = LightDesc;

    struct Sample {
        Evaluation eval;
        Float wi;
    };

protected:
    const LightType _type{LightType::Area};

public:
    explicit Light(const LightDesc &desc, LightType light_type)
        : Node(desc), _type(light_type) {}
};
}// namespace vision