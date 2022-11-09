//
// Created by Zero on 28/10/2022.
//

#include "bxdf.h"
#include "math/geometry.h"
#include "math/warp.h"
#include "core/constants.h"

namespace vision {

Float BxDF::PDF(Float3 wo, Float3 wi) const noexcept {
    return cosine_hemisphere_PDF(abs_cos_theta(wi));
}

BSDFEval BxDF::evaluate(Float3 wo, Float3 wi) const noexcept {
    return {f(wo, wi), PDF(wo, wi)};
}

Bool BxDF::safe(Float3 wo, Float3 wi) const noexcept {
    return same_hemisphere(wo, wi);
}

BSDFEval BxDF::safe_evaluate(Float3 wo, Float3 wi) const noexcept {
    BSDFEval ret;
    Bool s = safe(wo, wi);
    ret.f = select(s, f(wo, wi), make_float3(0.f));
    ret.pdf = select(s, PDF(wo, wi), 1.f);
    return ret;
}

Float3 LambertReflection::f(Float3 wo, Float3 wi) const noexcept {
    return Kr * InvPi * abs_cos_theta(wi);
}

BSDFSample BxDF::sample(Float3 wo, Float2 u) const noexcept {
    BSDFSample ret;
    ret.wi = square_to_cosine_hemisphere(u);
    ret.wi.z = select(wo.z < 0.f, -ret.wi.z, ret.wi.z);
    ret.eval = evaluate(wo, ret.wi);
    return ret;
}

Float3 MicrofacetReflection::f(Float3 wo, Float3 wi) const noexcept {
    Float cos_theta_o = cos_theta(wo);
    Float cos_theta_i = cos_theta(wi);
    Float3 wh = normalize(wo + wi);
    wh = face_forward(wh, make_float3(0, 0, 1));
    Float3 F = eval_fresnel(abs_dot(wo, wh), _fresnel_type);
    Float3 fr = _microfacet.BRDF(wo, wh, wi, F);
    return fr * Kr * abs_cos_theta(wi);
}

Float MicrofacetReflection::PDF(Float3 wo, Float3 wi) const noexcept {
    Float3 wh = normalize(wo + wi);
    return _microfacet.PDF_wi_reflection(wo, wh);
}

BSDFSample MicrofacetReflection::sample(Float3 wo, Float2 u) const noexcept {
    BSDFSample ret;
    Float3 wh = _microfacet.sample_wh(wo, u);
    Float3 wi = reflect(wo, wh);
    ret.eval = evaluate(wo, wi);
    ret.wi = wi;
    ret.flags = flag();
    return ret;
}
}// namespace vision