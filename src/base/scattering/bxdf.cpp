//
// Created by Zero on 28/10/2022.
//

#include "bxdf.h"
#include "math/geometry.h"
#include "math/warp.h"
#include "math/constants.h"

namespace vision {

// BxDF
Float BxDF::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    return cosine_hemisphere_PDF(abs_cos_theta(wi));
}

ScatterEval BxDF::evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel, MaterialEvalMode mode) const noexcept {
    ScatterEval ret{swl().dimension()};
    if (BxDF::match_F(mode)) {
        ret.f = f(wo, wi, fresnel);
    }
    if (BxDF::match_PDF(mode)) {
        ret.pdf = PDF(wo, wi, fresnel);
    }
    ret.flags = flags();
    return ret;
}

Bool BxDF::safe(Float3 wo, Float3 wi) const noexcept {
    return same_hemisphere(wo, wi);
}

ScatterEval BxDF::safe_evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel, MaterialEvalMode mode) const noexcept {
    ScatterEval ret{swl().dimension()};
    Bool s = safe(wo, wi);
    if (BxDF::match_F(mode)) {
        ret.f = select(s, f(wo, wi, fresnel), 0.f);
    }
    if (BxDF::match_PDF(mode)) {
        ret.pdf = select(s, PDF(wo, wi, fresnel), 0.f);
    }
    ret.flags = flags();
    return ret;
}

SampledDirection BxDF::sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wi = square_to_cosine_hemisphere(u);
    wi.z = select(wo.z < 0.f, -wi.z, wi.z);
    return {wi, 1.f};
}

BSDFSample BxDF::sample(Float3 wo, Sampler &sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl().dimension()};
    auto [wi, pdf] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.wi = wi;
    ret.eval = evaluate(wo, wi, fresnel, MaterialEvalMode::All);
    ret.eval.pdf *= pdf;
    return ret;
}

// MicrofacetReflection
SampledSpectrum MicrofacetReflection::f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    wh = face_forward(wh, make_float3(0, 0, 1));
    SampledSpectrum F = fresnel->evaluate(abs_dot(wo, wh));
    SampledSpectrum fr = microfacet_->BRDF(wo, wh, wi, F);
    return fr * kr_;
}

Float MicrofacetReflection::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    return microfacet_->PDF_wi_reflection(wo, wh);
}

SampledDirection MicrofacetReflection::sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, u);
    Float3 wi = reflect(wo, wh);
    return {wi, 1.f};
}

BSDFSample MicrofacetReflection::sample(Float3 wo, Sampler &sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl().dimension()};
    auto [wi, pdf] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.eval = safe_evaluate(wo, wi, fresnel, MaterialEvalMode::All);
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
    SampledSpectrum tr = microfacet_->BTDF(wo, wh, wi, SampledSpectrum(F.dimension(), 1.f) - F, eta);
    return select(same_hemisphere(wo, wi, wh), 0.f, tr * kt_);
}

Float MicrofacetTransmission::PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept {
    Float eta = fresnel->eta()[0];
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    return select(same_hemisphere(wo, wi, wh), 0.f, microfacet_->PDF_wi_transmission(wo, wh, wi, eta));
}

SampledDirection MicrofacetTransmission::sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, u);
    Float3 wi;
    Bool valid = refract(wo, wh, fresnel->eta()[0], addressof(wi));
    return {wi, valid && dot(wh, wo) > 0};
}

BSDFSample MicrofacetTransmission::sample(Float3 wo, Sampler &sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl().dimension()};
    auto [wi, valid] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.eval = safe_evaluate(wo, wi, fresnel, MaterialEvalMode::All);
    ret.eval.pdf = select(valid, ret.eval.pdf, 0.f);
    ret.wi = wi;
    ret.eta = fresnel->eta()[0];
    return ret;
}

}// namespace vision