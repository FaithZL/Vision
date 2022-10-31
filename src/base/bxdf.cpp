//
// Created by Zero on 28/10/2022.
//

#include "bxdf.h"
#include "math/geometry.h"
#include "math/warp.h"
#include "core/constants.h"

namespace vision {

Float BxDF::PDF(Float3 wo, Float3 wi) const noexcept {
    return cosine_hemisphere_PDF(wi.z);
}

Evaluation BxDF::evaluate(Float3 wo, Float3 wi) const noexcept {
    return {f(wo, wi), PDF(wo, wi)};
}

Bool BxDF::safe(Float3 wo, Float3 wi) const noexcept {
    return same_hemisphere(wo, wi);
}

Evaluation BxDF::safe_evaluate(Float3 wo, Float3 wi) const noexcept {
    Evaluation ret;
    Bool s = safe(wo, wi);
    ret.f = select(s, f(wo, wi), make_float3(0.f));
    ret.pdf = select(s, PDF(wo, wi), 0.f);
    return ret;
}

Float3 LambertReflection::f(Float3 wo, Float3 wi) const noexcept {
    return Kr * InvPi;
}

BSDFSample BxDF::sample(Float3 wo, Float2 u) const noexcept {
    BSDFSample ret;
    ret.wi = square_to_cosine_hemisphere(u);
    wo.z = select(wo.z < 0.f, -wo.z, wo.z);
    ret.eval = evaluate(wo, ret.wi);
    return ret;
}


}// namespace vision