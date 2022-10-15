//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"

namespace vision {
using namespace ocarina;
class Sampler : public Node {
public:
    using Desc = SamplerDesc;

protected:
    uint _spp{1u};

public:
    explicit Sampler(const SamplerDesc *desc) : Node(desc), _spp(desc->spp) {}
};
}// namespace vision