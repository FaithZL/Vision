//
// Created by ling.zhu on 2025/1/25.
//

#include "bxdf_set.h"

namespace vision {

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
    OC_NOT_IMPLEMENT_ERROR(BxDFSet::precompute_with_radio);
    return SampledSpectrum::zero(swl()->dimension());
}

void BxDFSet::from_ratio_x(const Float &x) noexcept {
    OC_ASSERT(0);
}

Float BxDFSet::to_ratio_x() noexcept {
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
}

Float BxDFSet::to_ratio_z() noexcept {
    OC_ASSERT(0);
    return 0;
}

uint64_t MicrofacetBxDFSet::_compute_type_hash() const noexcept {
    return hash64(fresnel_->type_hash(), refl_->type_hash());
}

MicrofacetBxDFSet::MicrofacetBxDFSet(const SP<Fresnel> &fresnel,
                                     UP<MicrofacetBxDF> refl)
    : fresnel_(fresnel), refl_(std::move(refl)) {}

void MicrofacetBxDFSet::from_ratio_x(const Float &roughness) noexcept {
    bxdf()->set_alpha(roughness);
}

Float MicrofacetBxDFSet::to_ratio_x() noexcept {
    Float ax = bxdf()->alpha_x();
    Float ay = bxdf()->alpha_y();
    return ocarina::clamp(ocarina::sqrt(ax * ay), 1e-4f, 1.0f);
}

void MicrofacetBxDFSet::from_ratio_z(ocarina::Float z) noexcept {
    Float ior = schlick_ior_from_F0(Pow<4>(z));
    fresnel_->set_eta(SampledSpectrum(bxdf()->swl().dimension(), ior));
}

Float MicrofacetBxDFSet::to_ratio_z() noexcept {
    Float ior = fresnel_->eta().average();
    return ocarina::sqrt(ocarina::abs((ior - 1.0f) / (ior + 1.0f)));
}

const SampledWavelengths *MicrofacetBxDFSet::swl() const {
    return &refl_->swl();
}

SampledSpectrum MicrofacetBxDFSet::principled_albedo(const Float &cos_theta) const noexcept {
    return refl_->principled_albedo(cos_theta) * fresnel_->evaluate(cos_theta);
}

ScatterEval MicrofacetBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              vision::MaterialEvalMode mode,
                                              const Uint &flag) const noexcept {
    return refl_->safe_evaluate(wo, wi, fresnel_->clone(), mode);
}

BSDFSample MicrofacetBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           vision::TSampler &sampler) const noexcept {
    return refl_->sample(wo, sampler, fresnel_->clone());
}

BSDFSample MicrofacetBxDFSet::sample_delta_local(const Float3 &wo,
                                                 TSampler &sampler) const noexcept {
    Float3 wi = make_float3(-wo.xy(), wo.z);
    BSDFSample ret{refl_->swl()};
    ret.wi = wi;
    ret.eval = refl_->evaluate(wo, wi, fresnel_->clone(), All);
    return ret;
}

SampledDirection MicrofacetBxDFSet::sample_wi(const Float3 &wo,
                                              const Uint &flag,
                                              TSampler &sampler) const noexcept {
    return refl_->sample_wi(wo, sampler->next_2d(), fresnel_->clone());
}

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

SampledSpectrum DielectricBxDFSet::principled_albedo(const Float &cos_theta) const noexcept {
    SampledSpectrum F = fresnel_->evaluate(cos_theta);
    return F * refl_.principled_albedo(cos_theta) + (1 - F) * trans_.principled_albedo(cos_theta);
}

ScatterEval DielectricBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                              MaterialEvalMode mode, const Uint &flag) const noexcept {
    ScatterEval ret{refl_.swl()};
    auto fresnel = fresnel_->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    SampledSpectrum frs = fresnel->evaluate(abs_cos_theta(wo));
    Float fr = frs[0];
    $if(same_hemisphere(wo, wi)) {
        ret = refl_.evaluate(wo, wi, fresnel, mode);
        ret.pdfs *= frs.values();
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
        ret.pdfs *= 1 - frs.values();
    };
    return ret;
}

SampledDirection DielectricBxDFSet::sample_wi(const Float3 &wo, const Uint &flag,
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
        ret.pdf = fr;
    }
    $else {
        ret = trans_.sample_wi(wo, sampler->next_2d(), fresnel);
        ret.pdf = 1 - fr;
    };
    return ret;
}

BSDFSample DielectricBxDFSet::sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept {
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

BSDFSample DielectricBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                           TSampler &sampler) const noexcept {
    BSDFSample ret{refl_.swl()};
    Float uc = sampler->next_1d();
    auto fresnel = fresnel_->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    SampledSpectrum frs = fresnel->evaluate(abs_cos_theta(wo));
    Float fr = frs[0];
    $if(uc < fr) {
        ret = refl_.sample(wo, sampler, fresnel);
        ret.eval.pdfs *= frs.values();
    }
    $else {
        ret = trans_.sample(wo, sampler, fresnel);
        if (trans_.swl().scatter_pdf_dim() > 1) {
            trans_.swl().foreach_secondary_channel([&](uint channel) {
                $if(frs[channel] == 1) {
                    trans_.swl().invalidation_channel(channel);
                };
            });
        }
        ret.eval.pdfs *= 1 - frs.values();
    };
    return ret;
}

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

SampledSpectrum MultiBxDFSet::principled_albedo(const Float &cos_theta) const noexcept {
    SampledSpectrum ret = SampledSpectrum::zero(swl()->dimension());
    for_each([&](const WeightedBxDFSet &lobe) {
        ret += lobe->principled_albedo(cos_theta);
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
    ret.eval.pdfs = select(sd.valid(), ret.eval.pdf() * sd.pdf, 0.f);
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