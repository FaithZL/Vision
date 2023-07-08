//
// Created by Zero on 28/10/2022.
//

#include "bxdf.h"
#include "math/geometry.h"
#include "math/warp.h"
#include "core/constants.h"

namespace vision {

// BxDF
Float BxDF::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    return cosine_hemisphere_PDF(abs_cos_theta(wi));
}

ScatterEval BxDF::evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    return {f(wo, wi, fresnel), PDF(wo, wi, fresnel), flags()};
}

Bool BxDF::safe(Float3 wo, Float3 wi) const noexcept {
    return same_hemisphere(wo, wi);
}

ScatterEval BxDF::safe_evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    ScatterEval ret{swl().dimension()};
    Bool s = safe(wo, wi);
    ret.f = select(s, f(wo, wi, fresnel), 0.f);
    ret.pdf = select(s, PDF(wo, wi, fresnel), 1.f);
    ret.flags = flags();
    return ret;
}

SampledDirection BxDF::sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wi = square_to_cosine_hemisphere(u);
    wi.z = select(wo.z < 0.f, -wi.z, wi.z);
    return {wi, 1.f};
}

BSDFSample BxDF::sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl().dimension()};
    auto [wi, pdf] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.wi = wi;
    ret.eval = evaluate(wo, wi, fresnel);
    ret.eval.pdf *= pdf;
    return ret;
}

// MicrofacetReflection
SampledSpectrum MicrofacetReflection::f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    wh = face_forward(wh, make_float3(0, 0, 1));
    SampledSpectrum F = fresnel->evaluate(abs_dot(wo, wh));
    SampledSpectrum fr = _microfacet->BRDF(wo, wh, wi, F);
    return fr * Kr;
}

Float MicrofacetReflection::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    return _microfacet->PDF_wi_reflection(wo, wh);
}

SampledDirection MicrofacetReflection::sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = _microfacet->sample_wh(wo, u);
    Float3 wi = reflect(wo, wh);
    return {wi, 1.f};
}

BSDFSample MicrofacetReflection::sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl().dimension()};
    auto [wi, pdf] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.eval = safe_evaluate(wo, wi, fresnel);
    ret.wi = wi;
    return ret;
}

// MicrofacetTransmission
Bool MicrofacetTransmission::safe(Float3 wo, Float3 wi) const noexcept {
    return !same_hemisphere(wo, wi);
}

SampledSpectrum MicrofacetTransmission::f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float eta = fresnel->eta()[0];
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    SampledSpectrum F = fresnel->evaluate(abs_dot(wo, wh));
    SampledSpectrum tr = _microfacet->BTDF(wo, wh, wi, SampledSpectrum(F.dimension(), 1.f) - F, eta);
    return select(dot(wo, wh) * dot(wi, wh) > 0, 0.f, tr * Kt);
}

Float MicrofacetTransmission::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float eta = fresnel->eta()[0];
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    return select(dot(wo, wh) * dot(wi, wh) > 0, 0.f, _microfacet->PDF_wi_transmission(wo, wh, wi, eta));
}

SampledDirection MicrofacetTransmission::sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = _microfacet->sample_wh(wo, u);
    auto [valid, wi] = refract(wo, wh, fresnel->eta()[0]);
    return {wi, valid && dot(wh, wo) > 0};
}

BSDFSample MicrofacetTransmission::sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl().dimension()};
    auto [wi, valid] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.eval = safe_evaluate(wo, wi, fresnel);
    ret.eval.pdf = select(valid, ret.eval.pdf, 0.f);
    ret.wi = wi;
    ret.eta = fresnel->eta()[0];
    return ret;
}

}// namespace vision