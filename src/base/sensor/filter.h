//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "base/node.h"
#include "descriptions/node_desc.h"

namespace vision {
using namespace ocarina;

struct FilterSample {
    Float2 p;
    Float weight;
};

//"filter": {
//    "type": "box",
//    "name": "boxFilter",
//    "param": {
//        "radius": [
//            0.5,
//            0.5
//        ]
//    }
//}
class Filter : public Node, public Serializable<float> {
public:
    using Desc = FilterDesc;

protected:
    Serial<float2> _radius;

public:
    explicit Filter(const FilterDesc &desc)
        : Node(desc),
          _radius(desc["radius"].as_float2(make_float2(1.f))) {}
    OC_SERIALIZABLE_FUNC(_radius)
    [[nodiscard]] virtual FilterSample sample(Float2 u) const noexcept = 0;
    [[nodiscard]] virtual float evaluate(float2 p) const noexcept = 0;
    [[nodiscard]] Float2 radius() const noexcept { return *_radius; }
};

}// namespace vision