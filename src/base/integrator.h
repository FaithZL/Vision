//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "node.h"
#include "rhi/common.h"
#include "interaction.h"
#include "lightsampler.h"
#include "base/scattering/material.h"
#include "sampler.h"
#include "math/warp.h"

namespace vision {

class RenderPipeline;

class ScatterFunction;

class Sampler;

class Integrator : public Node {
public:
    using Desc = IntegratorDesc;
    using signature = void(uint);

protected:
    uint _max_depth;
    uint _min_depth;
    float _rr_threshold;
    ocarina::Kernel<signature> _kernel;
    ocarina::Shader<signature> _shader;

public:
    explicit Integrator(const IntegratorDesc &desc)
        : Node(desc),
          _max_depth(desc.max_depth),
          _min_depth(desc.min_depth),
          _rr_threshold(desc.rr_threshold) {}
    virtual void compile_shader(RenderPipeline *rp) noexcept = 0;
    virtual void render(RenderPipeline *rp) const noexcept = 0;
};
}// namespace vision