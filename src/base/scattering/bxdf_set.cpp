//
// Created by ling.zhu on 2025/1/25.
//

#include "bxdf_set.h"
#include "material.h"
#include "precomputed_table.inl.h"

namespace vision {

/// BxDFSet
SampledSpectrum BxDFSet::precompute_albedo(const Float3 &wo, TSampler &sampler,
                                           const Uint &sample_num) noexcept {
    SampledSpectrum ret = SampledSpectrum::zero(3);
    $for(i, sample_num) {
        BSDFSample bs = sample_local(wo, BxDFFlag::All, sampler, Importance);
        ScatterEval se = bs.eval;
        $if(se.pdf() > 0) {
            auto r = se.throughput() * abs_cos_theta(bs.wi);
            ret += r;
        };
    };
    return ret / sample_num;
}

SampledSpectrum BxDFSet::precompute_with_radio(const Float3 &ratio, TSampler &sampler,
                                               const Uint &sample_num) noexcept {
    from_ratio_x(ratio.x);
    Float3 wo = from_ratio_y(ratio.y);
    from_ratio_z(ratio.z);
    return precompute_albedo(wo, sampler, sample_num);
}

Float BxDFSet::valid_factor(const Float3 &wo, const Float3 &wi) const noexcept {
    Bool valid = same_hemisphere(wo, wi);
    return cast<float>(valid);
}

BSDFSample BxDFSet::sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                 TransportMode tm) const noexcept {
    BSDFSample ret{*swl()};
    SampledDirection sd = sample_wi(wo, flag, sampler);
    ret.wi = sd.wi;
    ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag, tm, addressof(ret.eta));
    ret.eval.pdfs *= sd.factor();
    return ret;
}

void BxDFSet::from_ratio_x(const Float &x) noexcept {
    OC_ASSERT(0);
}

Float BxDFSet::to_ratio_x() const noexcept {
    OC_ASSERT(0);
    return 0;
}

Float3 BxDFSet::from_ratio_y(Float cos_theta) noexcept {
    cos_theta = ocarina::clamp(cos_theta, 1e-4f, 1.0f);
    Float z = cos_theta;
    Float y = 0;
    Float x = ocarina::sqrt(1 - sqr(z));
    return make_float3(x, y, z);
}

Float BxDFSet::to_ratio_y(const ocarina::Float3 &wo) noexcept {
    return abs_cos_theta(wo);
}

void BxDFSet::from_ratio_z(ocarina::Float z) noexcept {
    OC_ASSERT(0);
}

Float BxDFSet::to_ratio_z() const noexcept {
    OC_ASSERT(0);
    return 0;
}

/// MicrofacetBxDFSet
uint64_t MicrofacetBxDFSet::_compute_type_hash() const noexcept {
    return hash64(fresnel_->type_hash(), bxdf_->type_hash());
}

MicrofacetBxDFSet::MicrofacetBxDFSet(const SP<Fresnel> &fresnel,
                                     UP<MicrofacetBxDF> refl)
    : fresnel_(fresnel), bxdf_(std::move(refl)) {}

void MicrofacetBxDFSet::from_ratio_x(const Float &roughness) noexcept {
    bxdf()->set_alpha(clamp(sqr(roughness), alpha_lower, alpha_upper));
}

Float MicrofacetBxDFSet::to_ratio_x() const noexcept {
    Float ax = bxdf()->alpha_x();
    Float ay = bxdf()->alpha_y();
    Float a = sqrt(ax * ay);
    return sqrt(a);
}

namespace detail {
[[nodiscard]] Float to_ratio_z(const Float &ior) {
    return ocarina::sqrt(ocarina::abs((ior - 1.0f) / (ior + 1.0f)));
}
}// namespace detail

const SampledWavelengths *MicrofacetBxDFSet::swl() const {
    return &bxdf_->swl();
}

SampledSpectrum MicrofacetBxDFSet::albedo(const Float &cos_theta) const noexcept {
    return bxdf_->albedo(cos_theta) * fresnel_->evaluate(cos_theta);
}

ScatterEval MicrofacetBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              vision::MaterialEvalMode mode,
                                              const Uint &flag,
                                              TransportMode tm) const noexcept {
    return bxdf_->safe_evaluate(wo, wi, fresnel_.ptr(), mode, tm);
}

BSDFSample MicrofacetBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           vision::TSampler &sampler,
                                           TransportMode tm) const noexcept {
    return bxdf_->sample(wo, sampler, fresnel_.ptr(), tm);
}

BSDFSample MicrofacetBxDFSet::sample_delta_local(const Float3 &wo,
                                                 TSampler &sampler) const noexcept {
    Float3 wi = make_float3(-wo.xy(), wo.z);
    BSDFSample ret{bxdf_->swl()};
    ret.wi = wi;
    ret.eval = bxdf_->evaluate(wo, wi, fresnel_.ptr(), All, Radiance);
    return ret;
}

SampledDirection MicrofacetBxDFSet::sample_wi(const Float3 &wo,
                                              const Uint &flag,
                                              TSampler &sampler) const noexcept {
    return bxdf_->sample_wi(wo, sampler->next_2d(), fresnel_.ptr());
}

/// DiffuseBxDFSet
ScatterEval DiffuseBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                           MaterialEvalMode mode, const Uint &flag,
                                           TransportMode tm) const noexcept {
    return bxdf_->safe_evaluate(wo, wi, nullptr, mode, tm);
}

BSDFSample DiffuseBxDFSet::sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                        TransportMode tm) const noexcept {
    return bxdf_->sample(wo, sampler, nullptr, tm);
}

SampledDirection DiffuseBxDFSet::sample_wi(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept {
    return bxdf_->sample_wi(wo, sampler->next_2d(), nullptr);
}

/// BlackBodyBxDFSet
const SampledWavelengths *BlackBodyBxDFSet::swl() const {
    return swl_;
}

ScatterEval BlackBodyBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag,
                                             TransportMode tm) const noexcept {
    ScatterEval ret{*swl_};
    ret.f = {swl_->dimension(), 0.f};
    ret.pdfs = 1.f;
    return ret;
}

BSDFSample BlackBodyBxDFSet::sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                          TransportMode tm) const noexcept {
    BSDFSample ret{*swl_};
    ret.eval.pdfs = 1.f;
    /// Avoid sample discarding due to hemispherical check
    ret.eval.flags = BxDFFlag::DiffRefl;
    ret.wi = wo;
    return ret;
}


/// DielectricBxDFSet
void DielectricBxDFSet::prepare() noexcept {
    MaterialLut::instance().load_lut(lut_name, make_uint3(lut_res),
                                     PixelStorage::FLOAT2,
                                     addressof(DielectricBxDFSet_Table));

    MaterialLut::instance().load_lut(lut_inv_name, make_uint3(lut_res),
                                     PixelStorage::FLOAT2,
                                     addressof(DielectricInvBxDFSet_Table));
}

Uint DielectricBxDFSet::select_lut(const vision::SampledSpectrum &eta) noexcept {
    Uint idx = MaterialLut::instance().get_index(lut_name).hv();
    Uint inv_idx = MaterialLut::instance().get_index(lut_inv_name).hv();
    Uint index = ocarina::select(eta[0] > 1, idx, inv_idx);
    return index;
}

Float DielectricBxDFSet::eta_to_ratio_z(const Float &eta) noexcept {
    Float ret = ocarina::select(eta > 1.f, inverse_lerp(eta, ior_lower, ior_upper),
                                inverse_lerp(rcp(eta), ior_lower, ior_upper));
    return ret;
}

Float2 DielectricBxDFSet::sample_lut(const Float3 &wo, const SampledSpectrum &eta) const noexcept {
    Uint idx = select_lut(eta);
    const BindlessArray &ba = Global::instance().bindless_array();
    Float x = to_ratio_x();
    Float y = abs_cos_theta(wo);
    Float z = eta_to_ratio_z(eta[0]);
    Float3 uvw = make_float3(x, y, z);
    Float2 ret = ba.tex_var(idx).sample(2, uvw).as_vec2();
    return ret;
}

Float DielectricBxDFSet::refl_compensate(const Float3 &wo,
                                         const SampledSpectrum &eta) const noexcept {
    if (!compensate()) {
        return 1;
    }
    Float2 val = sample_lut(wo, eta);
    Float factor = val.x;
    Float refl = val.y;
    return rcp(factor);
}

Float DielectricBxDFSet::trans_compensate(const ocarina::Float3 &wo,
                                          const SampledSpectrum &eta) const noexcept {
    if (!compensate()) {
        return 1;
    }
    Float2 val = sample_lut(wo, eta);
    Float factor = val.x;
    return rcp(factor);
}

SampledSpectrum DielectricBxDFSet::albedo(const Float &cos_theta) const noexcept {
    SP<Fresnel> fresnel = fresnel_.ptr();
    SampledSpectrum eta = fresnel->eta();
    SampledSpectrum F = fresnel->evaluate(abs(cos_theta));
    return kt_ * (1 - F) + F;
}

ScatterEval DielectricBxDFSet::evaluate_impl(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                             const SP<const Fresnel> &fresnel, MaterialEvalMode mode,
                                             TransportMode tm) const noexcept {
    ScatterEval ret{*swl()};
    Bool reflect = same_hemisphere(wo, wi);
    SampledSpectrum F = fresnel->evaluate(abs_dot(wh, wo));
    $if(reflect) {
        ret = evaluate_reflection(wo, wh, wi, F, fresnel->eta(), mode);
    }
    $else {
        ret = evaluate_transmission(wo, wh, wi, F, fresnel->eta(), mode, tm);
    };
    return ret;
}

ScatterEval DielectricBxDFSet::evaluate_reflection(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                   const SampledSpectrum &F, const SampledSpectrum &eta,
                                                   MaterialEvalMode mode) const noexcept {
    ScatterEval se{*swl()};
    if (BxDF::match_F(mode)) {
        se.f = microfacet_->BRDF(wo, wh, wi, F);
    }
    if (BxDF::match_PDF(mode)) {
        se.pdfs = microfacet_->PDF_wi_reflection(wo, wh) * refl_prob(F);
    }
    se.flags = BxDFFlag::GlossyRefl;
    se.f *= refl_compensate(wo, eta);
    return se;
}

ScatterEval DielectricBxDFSet::evaluate_transmission(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                     const SampledSpectrum &F, const SampledSpectrum &eta,
                                                     MaterialEvalMode mode,
                                                     TransportMode tm) const noexcept {
    ScatterEval se{*swl()};
    Float3 new_wh = face_forward(wh, wo);
    if (BxDF::match_F(mode)) {
        SampledSpectrum tr = microfacet_->BTDF(wo, wi, (1 - F), eta[0], tm);
        se.f = tr * kt_;
    }
    if (BxDF::match_PDF(mode)) {
        se.pdfs = microfacet_->PDF_wi_transmission(wo, new_wh, wi, eta[0]) * trans_prob(F);
    }
    se.flags = BxDFFlag::GlossyTrans;
    se.f *= trans_compensate(wo, eta);
    return se;
}

Float DielectricBxDFSet::valid_factor(const Float3 &wo, const Float3 &wi) const noexcept {
    return 1.f;
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode, const Uint &flag,
                                              TransportMode tm) const noexcept {
    SP<Fresnel> fresnel = fresnel_.ptr();
    Bool reflect = same_hemisphere(wo, wi);
    Float eta_p = ocarina::select(reflect, 1.f, fresnel->eta()[0]);
    Float3 wh = normalize(wo + eta_p * wi);
    return evaluate_impl(wo, wh, wi, fresnel, mode, tm);
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                              const Uint &flag, TransportMode tm, Float *eta) const noexcept {
    SP<Fresnel> fresnel = fresnel_.ptr();
    if (eta) { *eta = fresnel->eta()[0]; }
    Bool reflect = same_hemisphere(wo, wi);
    Float eta_p = ocarina::select(reflect, 1.f, fresnel->eta()[0]);
    Float3 wh = normalize(wo + eta_p * wi);
    wh = face_forward(wh, wo);
    return evaluate_impl(wo, wh, wi, fresnel, mode, tm);
}

SampledDirection DielectricBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
                                              TSampler &sampler) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, sampler->next_2d());
    Float d = dot(wo, wh);
    auto fresnel = fresnel_.ptr();
    SampledDirection sd;
    SampledSpectrum F = fresnel->evaluate(abs(d));
    Float uc = sampler->next_1d();
    $if(uc < refl_prob(F)) {
        sd.wi = reflect(wo, wh);
        sd.valid = same_hemisphere(wo, sd.wi);
    }
    $else {
        Float eta = fresnel->eta()[0];
        Bool valid = refract(wo, wh, eta, &sd.wi);
        sd.valid = valid && !same_hemisphere(wo, sd.wi);
    };
    return sd;
}

BSDFSample DielectricBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler,
                                           TransportMode tm) const noexcept {
    return BxDFSet::sample_local(wo, flag, sampler, tm);
}

// PureReflectionBxDFSet
Float PureReflectionBxDFSet::compensate_factor(const ocarina::Float3 &wo) const noexcept {
    Float alpha = bxdf()->alpha_average();
    Float ret = MaterialLut::instance().sample(lut_name, 1, make_float2(alpha, cos_theta(wo))).as_scalar();
    return 1.f / ret;
}

void PureReflectionBxDFSet::prepare() {
    MaterialLut::instance().load_lut(lut_name, make_uint2(lut_res),
                                     PixelStorage::FLOAT1,
                                     addressof(PureReflectionBxDFSet_Table));
}

}// namespace vision