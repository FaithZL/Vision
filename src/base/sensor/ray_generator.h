//
// Created by ling.zhu on 2025/6/27.
//

#pragma once

#include "base/node.h"
#include "base/sample.h"
#include "math/transform.h"
#include "filter.h"
#include "hotfix/hotfix.h"

namespace vision {
using namespace ocarina;

class RayGenerator : public Node, public Observer {
public:
    using Desc = RayGeneratorDesc;

public:
    RayGenerator() = default;
    explicit RayGenerator(const RayGeneratorDesc &desc)
        : Node(desc) {}

    [[nodiscard]] virtual uint2 resolution() const noexcept = 0;
};

}// namespace vision