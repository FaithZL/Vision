//
// Created by Zero on 28/10/2022.
//

#include "material.h"
#include "base/sampler.h"

namespace vision {

Uchar BSDF::combine_flag(Float3 wo, Float3 wi, Uchar flag) noexcept {
    Bool reflect = same_hemisphere(wo, wi);
    uchar non_reflect = ~BxDFFlag::Reflection;
    uchar non_trans = ~BxDFFlag::Transmission;
    return select(reflect, flag & non_trans, flag & non_reflect);
}

ScatterEval BSDF::evaluate(Float3 world_wo, Float3 world_wi) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, BxDFFlag::All);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

BSDFSample BSDF::sample(Float3 world_wo, Sampler *sampler) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    BSDFSample ret;
    ret = sample_local(wo, BxDFFlag::All, sampler);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame.to_world(ret.wi);
    return ret;
}

ScatterEval DielectricBSDF::evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept {
    ScatterEval ret;
    auto fresnel = _fresnel->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    $if(same_hemisphere(wo, wi)) {
        ret = _refl.evaluate(wo, wi, fresnel);
    }
    $else {
        ret = _trans.evaluate(wo, wi, fresnel);
    };
    return ret;
}

BSDFSample DielectricBSDF::sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept {
    BSDFSample ret;
    Float uc = sampler->next_1d();
    auto fresnel = _fresnel->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
    $if(uc < fr) {
        ret = _refl.sample(wo, sampler, fresnel);
        ret.eval.pdf *= fr;
    }
    $else {
        ret = _trans.sample(wo, sampler, fresnel);
        ret.eval.pdf *= 1 - fr;
    };
    return ret;
}

}// namespace vision