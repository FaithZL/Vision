//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"

namespace vision {
class LightSampler : public Node {
public:
    using Desc = LightSamplerDesc;

public:
    explicit LightSampler(const LightSamplerDesc *desc) : Node(desc) {}
};
}// namespace vision