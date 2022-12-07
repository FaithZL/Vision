//
// Created by Zero on 07/12/2022.
//

#include "integrator.h"
#include "lightsampler.h"
#include "material.h"
#include "sampler.h"
#include "math/warp.h"

namespace vision {

Float3 Integrator::direct_lighting(Interaction it, const ScatterFunction &sf, LightSample ls,
                                   Bool occluded, Sampler *sampler, BSDFSample &ss) {
    Float3 wi = normalize(ls.p_light - it.pos);
    ScatterEval scatter_eval = sf.evaluate(wi);
    ss = *std::dynamic_pointer_cast<BSDFSample>(sf.sample(sampler));

    Bool is_delta_light = ls.eval.pdf < 0;
    Float weight = select(is_delta_light, 1.f, mis_weight<D>(ls.eval.pdf, scatter_eval.pdf));
    ls.eval.pdf = select(is_delta_light, -ls.eval.pdf, ls.eval.pdf);
    Float3 Ld = make_float3(0.f);
    $if(!occluded && scatter_eval.valid() && ls.valid()) {
        Ld = ls.eval.L * scatter_eval.f * weight / ls.eval.pdf;
    };
    return Ld;
}

}// namespace vision