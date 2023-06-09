//
// Created by Zero on 2023/5/30.
//

#pragma once

#include "dsl/common.h"
#include "node.h"

namespace vision {

enum DenoiseMode {
    RT = 0,
    RTLightmap = 1
};

class Denoiser : public Node {
protected:
    DenoiseMode _mode{};

public:
    using Desc = DenoiserDesc;

public:
    explicit Denoiser(const DenoiserDesc &desc) : Node(desc),_mode(RT) {}
    virtual void apply(uint2 res, float4 *output, float4 *color,
                       float4 *normal, float4 *albedo) const noexcept = 0;
};

}// namespace vision