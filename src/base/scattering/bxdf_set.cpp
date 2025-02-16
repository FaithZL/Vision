//
// Created by ling.zhu on 2025/1/25.
//

#include "bxdf_set.h"

namespace vision {

/// BxDFSet
SampledSpectrum BxDFSet::precompute_albedo(const Float3 &wo, TSampler &sampler,
                                           const Uint &sample_num) noexcept {
    SampledSpectrum ret = SampledSpectrum::zero(3);
    $for(i, sample_num) {
        BSDFSample bs = sample_local(wo, BxDFFlag::All, sampler);
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

BSDFSample BxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                 TSampler &sampler) const noexcept {
    BSDFSample ret{*swl()};
    SampledDirection sd = sample_wi(wo, flag, sampler);
    ret.wi = sd.wi;
    ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag);
    ret.eval.pdfs = ret.eval.pdf() * sd.factor();
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
    bxdf()->set_alpha(clamp(roughness, alpha_lower, alpha_upper));
}

Float MicrofacetBxDFSet::to_ratio_x() const noexcept {
    Float ax = bxdf()->alpha_x();
    Float ay = bxdf()->alpha_y();
    return ocarina::clamp(ocarina::sqrt(ax * ay), alpha_lower, alpha_upper);
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
                                              const Uint &flag) const noexcept {
    return bxdf_->safe_evaluate(wo, wi, fresnel_->clone(), mode);
}

BSDFSample MicrofacetBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           vision::TSampler &sampler) const noexcept {
    return bxdf_->sample(wo, sampler, fresnel_->clone());
}

BSDFSample MicrofacetBxDFSet::sample_delta_local(const Float3 &wo,
                                                 TSampler &sampler) const noexcept {
    Float3 wi = make_float3(-wo.xy(), wo.z);
    BSDFSample ret{bxdf_->swl()};
    ret.wi = wi;
    ret.eval = bxdf_->evaluate(wo, wi, fresnel_->clone(), All);
    return ret;
}

SampledDirection MicrofacetBxDFSet::sample_wi(const Float3 &wo,
                                              const Uint &flag,
                                              TSampler &sampler) const noexcept {
    return bxdf_->sample_wi(wo, sampler->next_2d(), fresnel_->clone());
}

/// DiffuseBxDFSet
ScatterEval DiffuseBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                           MaterialEvalMode mode, const Uint &flag) const noexcept {
    return bxdf_->safe_evaluate(wo, wi, nullptr, mode);
}

BSDFSample DiffuseBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                        TSampler &sampler) const noexcept {
    return bxdf_->sample(wo, sampler, nullptr);
}

SampledDirection DiffuseBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler) const noexcept {
    return bxdf_->sample_wi(wo, sampler->next_2d(), nullptr);
}

/// BlackBodyBxDFSet
const SampledWavelengths *BlackBodyBxDFSet::swl() const {
    return swl_;
}

ScatterEval BlackBodyBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode,
                                             const Uint &flag) const noexcept {
    ScatterEval ret{*swl_};
    ret.f = {swl_->dimension(), 0.f};
    ret.pdfs = 1.f;
    return ret;
}

BSDFSample BlackBodyBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept {
    BSDFSample ret{*swl_};
    ret.eval.pdfs = 1.f;
    /// Avoid sample discarding due to hemispherical check
    ret.eval.flags = BxDFFlag::DiffRefl;
    ret.wi = wo;
    return ret;
}

/// DielectricBxDFSetOld
SampledSpectrum DielectricBxDFSetOld::albedo(const Float &cos_theta) const noexcept {
    SampledSpectrum F = fresnel_->evaluate(cos_theta);
    return F * refl_.albedo(cos_theta) + (1 - F) * trans_.albedo(cos_theta);
}

ScatterEval DielectricBxDFSetOld::evaluate_local(const Float3 &wo, const Float3 &wi,
                                                 MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{refl_.swl()};
    auto fresnel = fresnel_->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    SampledSpectrum frs = fresnel->evaluate(abs_cos_theta(wo));
    Float fr = frs[0];
    $if(same_hemisphere(wo, wi)) {
        ret = refl_.evaluate(wo, wi, fresnel, mode);
        ret.pdfs *= frs.values()[0];
    }
    $else {
        ret = trans_.evaluate(wo, wi, fresnel, mode);
        if (trans_.swl().scatter_pdf_dim() > 1) {
            trans_.swl().foreach_secondary_channel([&](uint channel) {
                $if(frs[channel] == 1) {
                    trans_.swl().invalidation_channel(channel);
                };
            });
        }
        ret.pdfs *= 1 - frs.values()[0];
    };
    return ret;
}

SampledDirection DielectricBxDFSetOld::sample_wi(const Float3 &wo, const Uint &flag,
                                                 TSampler &sampler) const noexcept {
    Float uc = sampler->next_1d();
    auto fresnel = fresnel_->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    SampledSpectrum frs = fresnel->evaluate(abs_cos_theta(wo));
    Float fr = frs[0];
    SampledDirection ret;
    $if(uc < fr) {
        ret = refl_.sample_wi(wo, sampler->next_2d(), fresnel);
    }
    $else {
        ret = trans_.sample_wi(wo, sampler->next_2d(), fresnel);
    };
    return ret;
}

BSDFSample DielectricBxDFSetOld::sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept {
    BSDFSample ret{refl_.swl()};
    Float uc = sampler->next_1d();
    auto fresnel = fresnel_->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
    $if(uc < fr) {
        Float3 wi = wo;
        wi.xy() = -wi.xy();
        ret.wi = wi;
        ret.eval = refl_.evaluate(wo, wi, fresnel, All);
        ret.eval.pdfs *= fr;
    }
    $else {
        Float3 wi;
        Float3 n = face_forward(make_float3(0, 0, 1), wo);
        refract(wo, n, fresnel->eta()[0], &wi);
        ret.eval = trans_.evaluate(wo, wi, fresnel, All);
        ret.wi = wi;
        ret.eval.pdfs *= 1 - fr;
    };
    return ret;
}

/// DielectricBxDFSet
SampledSpectrum DielectricBxDFSet::albedo(const Float &cos_theta) const noexcept {
    return kt_;
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{*swl()};
    Float cos_theta_o = cos_theta(wo);
    Float cos_theta_i = cos_theta(wi);
    auto fresnel = fresnel_->clone();
    fresnel->correct_eta(cos_theta_o);
    Bool reflect = cos_theta_i * cos_theta_o;
//    $condition_info("{} {}", cos_theta_i, cos_theta_o);
    $if(reflect) {
        Float3 wh = normalize(wo + wi);
        SampledSpectrum F = fresnel->evaluate(dot(wo, wh));
        SampledSpectrum fr = microfacet_->BRDF(wo, wh, wi, F);
        ret.f = fr;
        ret.pdfs = microfacet_->PDF_wi_reflection(wo, wh);
        ret.flags = BxDFFlag::GlossyRefl;
    }
    $else{
        Float eta = fresnel->eta()[0];
        Float3 wh = normalize(wo + wi * eta);
        wh = face_forward(wh, make_float3(0, 0, 1));
        Float F = fresnel->evaluate(abs_dot(wo, wh), 0);
        Float tr = microfacet_->BTDF(wo, wh, wi, 1 - F, eta);
        ret.f = tr * kt_;
        ret.pdfs = microfacet_->PDF_wi_transmission(wo, wh, wi, eta);
        ret.flags = BxDFFlag::GlossyTrans;
//        $condition_info("{} {} {} --- {}", wo, tr);

    };
    return ret;
}

SampledDirection DielectricBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
                                              TSampler &sampler) const noexcept {
    Float3 wh = microfacet_->sample_wh(wo, sampler->next_2d());
    Float d = dot(wo, wh);
    auto fresnel = fresnel_->clone();
    fresnel->correct_eta(d);
    SampledSpectrum frs = fresnel->evaluate(abs(d));

    SampledDirection sd;
    $condition_info("wo {} {} {},   wh {} {} {} ", wo, wh);

    Float uc = sampler->next_1d();
    $if(uc < frs[0]) {
        wh = ocarina::select(d < 0.f, -wh, wh);
        sd.wi = reflect(wo, wh);
        sd.valid = same_hemisphere(wo, sd.wi);
    }
    $else {
//        wh = ocarina::select(d > 0.f, -wh, wh);
        Float eta = fresnel->eta()[0];
        sd.valid = refract(wo, wh, eta, &sd.wi);
        $condition_info(" refract  {} {} {} --- {} {} {}", wo, sd.wi);
    };
    return sd;
}

/// MultiBxDFSet
void MultiBxDFSet::for_each(const std::function<void(const WeightedBxDFSet &)> &func) const {
    std::for_each(lobes_.begin(), lobes_.end(), func);
}

void MultiBxDFSet::for_each(const std::function<void(WeightedBxDFSet &)> &func) {
    std::for_each(lobes_.begin(), lobes_.end(), func);
}

void MultiBxDFSet::for_each(const std::function<void(const WeightedBxDFSet &, uint)> &func) const {
    for (int i = 0; i < lobe_num(); ++i) {
        func(lobes_[i], i);
    }
}

void MultiBxDFSet::for_each(const std::function<void(WeightedBxDFSet &, uint)> &func) {
    for (int i = 0; i < lobe_num(); ++i) {
        func(lobes_[i], i);
    }
}

void MultiBxDFSet::normalize_weights() noexcept {
    Float weight_sum = 0;
    for_each([&](WeightedBxDFSet &lobe) {
        weight_sum += lobe.weight();
    });
    for_each([&](WeightedBxDFSet &lobe) {
        lobe.weight() = lobe.weight() / weight_sum;
    });
}

SampledSpectrum MultiBxDFSet::albedo(const Float &cos_theta) const noexcept {
    SampledSpectrum ret = SampledSpectrum::zero(swl()->dimension());
    for_each([&](const WeightedBxDFSet &lobe) {
        ret += lobe->albedo(cos_theta);
    });
    return ret;
}

SampledDirection MultiBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
                                         TSampler &sampler) const noexcept {
    Float uc = sampler->next_1d();
    Float2 u = sampler->next_2d();
    SampledDirection sd;
    Uint sampling_strategy = 0u;
    Float sum_weights = 0.f;

    for_each([&](const WeightedBxDFSet &lobe, uint i) {
        sampling_strategy = select(uc > sum_weights, i, sampling_strategy);
        sum_weights += lobe.weight();
    });
    if (lobe_num() == 1) {
        sd = lobes_[0]->sample_wi(wo, flag, sampler);
    } else {
        $switch(sampling_strategy) {
            for_each([&](const WeightedBxDFSet &lobe, uint i) {
                $case(i) {
                    sd = lobe->sample_wi(wo, flag, sampler);
                    $break;
                };
            });
            $default {
                unreachable();
                $break;
            };
        };
    }
    return sd;
}

BSDFSample MultiBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                      TSampler &sampler) const noexcept {
    BSDFSample ret{*swl()};
    SampledDirection sd = sample_wi(wo, flag, sampler);
    ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag);
    ret.wi = sd.wi;
    ret.eval.pdfs = ret.eval.pdfs * sd.factor();
    return ret;
}

Uint MultiBxDFSet::flag() const noexcept {
    Uint ret = BxDFFlag::Unset;
    for_each([&](const WeightedBxDFSet &lobe) {
        ret |= lobe->flag();
    });
    return ret;
}

ScatterEval MultiBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                         MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{*swl()};
    for_each([&](const WeightedBxDFSet &lobe) {
        ScatterEval se = lobe->evaluate_local(wo, wi, mode, flag);
        ret.f += se.f;
        ret.pdfs += se.pdfs * lobe.weight();
        ret.flags = ret.flags | lobe->flag();
    });
    return ret;
}

}// namespace vision