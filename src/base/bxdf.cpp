//
// Created by Zero on 28/10/2022.
//

#include "bxdf.h"
#include "math/geometry.h"
#include "math/warp.h"
#include "core/constants.h"

namespace vision {

Float BxDF::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    return cosine_hemisphere_PDF(abs_cos_theta(wi));
}

BSDFEval BxDF::evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    return {f(wo, wi, fresnel), PDF(wo, wi, fresnel)};
}

Bool BxDF::safe(Float3 wo, Float3 wi) const noexcept {
    return same_hemisphere(wo, wi);
}

BSDFEval BxDF::safe_evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    BSDFEval ret;
    Bool s = safe(wo, wi);
    ret.f = select(s, f(wo, wi, fresnel), make_float3(0.f));
    ret.pdf = select(s, PDF(wo, wi, fresnel), 1.f);
    return ret;
}

Float3 LambertReflection::f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    return Kr * InvPi * abs_cos_theta(wi);
}

BSDFSample BxDF::sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret;
    ret.wi = square_to_cosine_hemisphere(u);
    ret.wi.z = select(wo.z < 0.f, -ret.wi.z, ret.wi.z);
    ret.eval = evaluate(wo, ret.wi, fresnel);
    return ret;
}

Float3 MicrofacetReflection::f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    wh = face_forward(wh, make_float3(0, 0, 1));
    Float3 F = fresnel->evaluate(cos_theta(wi));
    Float3 fr = _microfacet->BRDF(wo, wh, wi, F);
    return fr * Kr * abs_cos_theta(wi);
}

Float MicrofacetReflection::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    return _microfacet->PDF_wi_reflection(wo, wh);
}

BSDFSample MicrofacetReflection::sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret;
    Float3 wh = _microfacet->sample_wh(wo, u);
    Float3 wi = reflect(wo, wh);
    ret.eval = safe_evaluate(wo, wi, fresnel);
    ret.wi = wi;
    ret.flags = flag();
    return ret;
}

Bool MicrofacetTransmission::safe(Float3 wo, Float3 wi) const noexcept {
    return !same_hemisphere(wo, wi);
}

Float3 MicrofacetTransmission::f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float eta = fresnel->eta();
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    Float3 F = fresnel->evaluate(abs_dot(wo, wh));
    Float3 tr = _microfacet->BTDF(wo, wh, wi, make_float3(1) - F, eta);
    return select(dot(wo, wh) * dot(wi, wh) > 0, make_float3(0.f), tr * Kt * abs_cos_theta(wi));
}

Float MicrofacetTransmission::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float eta = fresnel->eta();
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    return select(dot(wo, wh) * dot(wi, wh) > 0, 0.f, _microfacet->PDF_wi_transmission(wo, wh, wi, eta));
}

BSDFSample MicrofacetTransmission::sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret;
    Float3 wh = _microfacet->sample_wh(wo, u);
    auto [valid, wi] = refract(wo, wh, fresnel->eta());
    ret.eval = safe_evaluate(wo, wi, fresnel);
    ret.eval.pdf = select(valid && dot(wh, wo) > 0, ret.eval.pdf, 0.f);
    ret.wi = wi;
    ret.flags = flag();
    return ret;
}
}// namespace vision