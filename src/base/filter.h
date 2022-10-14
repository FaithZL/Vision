//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "descriptions/node_desc.h"

namespace vision {
using namespace ocarina;

struct FilterSample {
    Float2 p;
    Float weight;
};

class Filter : public Node {
public:
    using Desc = FilterDesc;

protected:
    float2 _radius;

public:
    explicit Filter(const FilterDesc *desc) : Node(desc), _radius(desc->radius) {}
    [[nodiscard]] virtual FilterSample sample(Float2 u) const noexcept = 0;
    [[nodiscard]] virtual float evaluate(float2 p) const noexcept = 0;
};

}// namespace vision