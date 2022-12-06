//
// Created by Zero on 28/10/2022.
//

#include "material.h"
#include "sampler.h"

namespace vision {

Uchar BSDF::combine_flag(Float3 wo, Float3 wi, Uchar flag) noexcept {
    Bool reflect = same_hemisphere(wo, wi);
    uchar non_reflect = ~BxDFFlag::Reflection;
    uchar non_trans = ~BxDFFlag::Transmission;
    return select(reflect, flag & non_trans, flag & non_reflect);
}

BSDFSample BSDF::sample(Float uc, Float2 u, Uchar flag) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    BSDFSample ret = sample_local(wo, uc, u, flag);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame.to_world(ret.wi);
    return ret;
}

ScatterEval BSDF::evaluate(Float3 world_wi, Uchar flag) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, flag);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

ScatterEval BSDF::evaluate(Float3 world_wi) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, BxDFFlag::All);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

SP<ScatterSample> BSDF::sample(Sampler *sampler) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    SP<ScatterSample> ret = sample_local(wo, sampler);
    ret->eval.f *= abs_cos_theta(ret->wi);
    ret->wi = shading_frame.to_world(ret->wi);
    return ret;
}

}// namespace vision