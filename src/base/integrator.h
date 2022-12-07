//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "node.h"
#include "rhi/common.h"
#include "interaction.h"
#include "lightsampler.h"
#include "material.h"
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

    template<typename SF, typename SS>
    static Float3 direct_lighting(Interaction it, const SF &sf, LightSample ls,
                                 Bool occluded, Sampler *sampler,SS &ss) {
        Float3 wi = normalize(ls.p_light - it.pos);
        ScatterEval scatter_eval = sf.evaluate(it.wo, wi);
        ss = sf.sample(it.wo, sampler);
        Bool is_delta_light = ls.eval.pdf < 0;
        Float weight = select(is_delta_light, 1.f, mis_weight<D>(ls.eval.pdf, scatter_eval.pdf));
        ls.eval.pdf = select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
        Float3 Ld = make_float3(0.f);
        $if(!occluded && scatter_eval.valid() && ls.valid()) {
            Ld = ls.eval.L * scatter_eval.f * weight / ls.eval.pdf;
        };
        return Ld;
    }
};
}// namespace vision