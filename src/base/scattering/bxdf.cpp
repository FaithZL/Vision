//
// Created by Zero on 28/10/2022.
//

#include "bxdf.h"
#include "math/geometry.h"
#include "math/warp.h"
#include "math/constants.h"

namespace vision {

// BxDF
Float BxDF::PDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    return cosine_hemisphere_PDF(abs_cos_theta(wi));
}

ScatterEval BxDF::evaluate(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel, MaterialEvalMode mode) const noexcept {
    ScatterEval ret{swl()};
    if (BxDF::match_F(mode)) {
        ret.f = f(wo, wi, fresnel);
    }
    if (BxDF::match_PDF(mode)) {
        ret.pdfs = PDF(wo, wi, fresnel);
    }
    ret.flags = flags();
    return ret;
}

Bool BxDF::safe(const Float3 &wo, const Float3 &wi) const noexcept {
    return same_hemisphere(wo, wi);
}

ScatterEval BxDF::safe_evaluate(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel, MaterialEvalMode mode) const noexcept {
    ScatterEval ret{swl()};
    Bool s = safe(wo, wi);
    if (BxDF::match_F(mode)) {
        ret.f = select(s, f(wo, wi, fresnel), 0.f);
    }
    if (BxDF::match_PDF(mode)) {
        ret.pdfs = select(s, PDF(wo, wi, fresnel), 0.f);
    }
    ret.flags = flags();
    return ret;
}

SampledDirection BxDF::sample_wi(const Float3 &wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wi = square_to_cosine_hemisphere(u);
    wi.z = select(wo.z < 0.f, -wi.z, wi.z);
    return {wi, 1.f};
}

BSDFSample BxDF::sample(const Float3 &wo, TSampler &sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl()};
    auto [wi, pdf] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.wi = wi;
    ret.eval = evaluate(wo, wi, fresnel, MaterialEvalMode::All);
    ret.eval.pdfs *= pdf;
    return ret;
}

// MicrofacetReflection
SampledSpectrum MicrofacetReflection::f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    wh = face_forward(wh, make_float3(0, 0, 1));
    SampledSpectrum F = fresnel->evaluate(abs_dot(wo, wh));
    SampledSpectrum fr = microfacet_->BRDF(wo, wh, wi, F);
    return fr * kr_;
}

Float MicrofacetReflection::PDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = normalize(wo + wi);
    return microfacet_->PDF_wi_reflection(wo, wh);
}

SampledDirection MicrofacetReflection::sample_wi(const Float3 &wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, u);
    Float3 wi = reflect(wo, wh);
    return {wi, 1.f};
}

BSDFSample MicrofacetReflection::sample(const Float3 &wo, TSampler &sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl()};
    auto [wi, pdf] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.eval = safe_evaluate(wo, wi, fresnel, MaterialEvalMode::All);
    ret.wi = wi;
    return ret;
}

// MicrofacetTransmission
Bool MicrofacetTransmission::safe(const Float3 &wo, const Float3 &wi) const noexcept {
    return !same_hemisphere(wo, wi);
}

Float MicrofacetTransmission::BTDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel, uint channel) const noexcept {
    SampledSpectrum etas = fresnel->eta();
    Float eta = etas[channel];
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    Float F = fresnel->evaluate(abs_dot(wo, wh), channel);
    Float tr = microfacet_->BTDF(wo, wh, wi, 1 - F, eta);
    return tr;
}

SampledSpectrum MicrofacetTransmission::BTDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    uint dim = fresnel->eta().dimension();
    SampledSpectrum ret{dim};
    ret.foreach_dimensions([&](uint i) {
        ret[i] = BTDF(wo, wi, fresnel, i);
    });
    return ret;
}

SampledSpectrum MicrofacetTransmission::f_array(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    uint dim = fresnel->eta().dimension();
    return BTDF(wo, wi, fresnel) * kt_;
}

SampledSpectrum MicrofacetTransmission::f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    SampledSpectrum etas = fresnel->eta();
    Float eta = etas[0];
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    SampledSpectrum F = fresnel->evaluate(abs_dot(wo, wh));
    SampledSpectrum tr = microfacet_->BTDF(wo, wh, wi, SampledSpectrum(F.dimension(), 1.f) - F, eta);
    return select(same_hemisphere(wo, wi, wh), 0.f, tr * kt_);
}

Float MicrofacetTransmission::PDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel, uint channel) const noexcept {
    SampledSpectrum etas = fresnel->eta();
    Float eta = etas[channel];
    Float3 wh = normalize(wo + wi * eta);
    wh = face_forward(wh, make_float3(0, 0, 1));
    return select(same_hemisphere(wo, wi, wh), 0.f, microfacet_->PDF_wi_transmission(wo, wh, wi, eta));
}

Float MicrofacetTransmission::PDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    return PDF(wo, wi, fresnel, 0);
}

float_array MicrofacetTransmission::PDF_array(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel) const noexcept {
    uint dim = fresnel->eta().dimension();
    float_array ret = float_array::zero(dim);
    ret.foreach ([&](uint i) {
        ret[i] = PDF(wo, wi, fresnel, i);
    });
    return ret;
}

SampledDirection MicrofacetTransmission::sample_wi(const Float3 &wo, Float2 u, SP<Fresnel> fresnel) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, u);
    Float3 wi;
    Bool valid = refract(wo, wh, fresnel->eta()[0], addressof(wi));
    return {wi, valid && dot(wh, wo) > 0};
}

ScatterEval MicrofacetTransmission::safe_evaluate(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel, MaterialEvalMode mode) const noexcept {
    //    return BxDF::safe_evaluate(wo, wi, fresnel, mode);
    ScatterEval ret{swl()};
    Bool s = safe(wo, wi);
    if (BxDF::match_F(mode)) {
        ret.f = select(s, f_array(wo, wi, fresnel), 0.f);
    }
    if (BxDF::match_PDF(mode)) {
        ret.pdfs = select(s, PDF_array(wo, wi, fresnel), 0.f);
    }
    ret.flags = flags();
    return ret;
}

BSDFSample MicrofacetTransmission::sample(const Float3 &wo, TSampler &sampler, SP<Fresnel> fresnel) const noexcept {
    BSDFSample ret{swl()};
    auto [wi, valid] = sample_wi(wo, sampler->next_2d(), fresnel);
    ret.eval = safe_evaluate(wo, wi, fresnel, MaterialEvalMode::All);
    ret.eval.pdfs = select(valid, ret.eval.pdf(), 0.f);
    ret.wi = wi;
    ret.eta = fresnel->eta()[0];
    return ret;
}

OrenNayar::OrenNayar(SampledSpectrum R, Float sigma,
                     const SampledWavelengths &swl)
: BxDF(swl, BxDFFlag::DiffRefl), R_(std::move(R)) {
        sigma = sigma * constants::PiOver2;
        Float sigma2 = ocarina::sqr(sigma * sigma);
        A_ = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
        B_ = 0.45f * sigma2 / (sigma2 + 0.09f);
    }

SampledSpectrum OrenNayar::f(const Float3 &wo, const Float3 &wi,
                             SP<Fresnel> fresnel) const noexcept  {
    Float sin_theta_i = sin_theta(wi);
    Float sin_theta_o = sin_theta(wo);

    Float sin_phi_i = sin_phi(wi);
    Float cos_phi_i = cos_phi(wi);
    Float sin_phi_o = sin_phi(wo);
    Float cos_phi_o = cos_phi(wo);
    Float d_cos = cos_phi_i * cos_phi_o + sin_phi_i * sin_phi_o;

    Float max_cos = ocarina::max(0.f, d_cos);

    Bool cond = abs_cos_theta(wi) > abs_cos_theta(wo);
    Float sin_alpha = select(cond, sin_theta_o, sin_theta_i);
    Float tan_beta = select(cond, sin_theta_i / abs_cos_theta(wi),
                            sin_theta_o / abs_cos_theta(wo));
    return R_ * InvPi * (A_ + B_ * max_cos * sin_alpha * tan_beta);
}

}// namespace vision