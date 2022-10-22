//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "node.h"

namespace vision {

class RenderPipeline;

class Integrator : public Node {
public:
    using Desc = IntegratorDesc;

protected:
    uint _max_depth;
    uint _min_depth;
    float _rr_threshold;

public:
    explicit Integrator(const IntegratorDesc &desc)
        : Node(desc),
          _max_depth(desc.max_depth),
          _min_depth(desc.min_depth),
          _rr_threshold(desc.rr_threshold) {}

    virtual void render(RenderPipeline *rp) const noexcept = 0;
};
}// namespace vision