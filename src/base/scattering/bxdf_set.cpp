//
// Created by ling.zhu on 2025/1/25.
//

#include "bxdf_set.h"

namespace vision {

uint64_t UniversalReflectBxDFSet::_compute_type_hash() const noexcept {
    return hash64(fresnel_->type_hash(), refl_->type_hash());
}

UniversalReflectBxDFSet::UniversalReflectBxDFSet(const SP<vision::Fresnel> &fresnel,
                                                 UP<vision::BxDF> refl)
    : fresnel_(fresnel), refl_(std::move(refl)) {}

const SampledWavelengths *UniversalReflectBxDFSet::swl() const {
    return &refl_->swl();
}

SampledSpectrum UniversalReflectBxDFSet::albedo(const ocarina::Float3 &wo) const noexcept {
    return refl_->albedo(wo);
}

ScatterEval UniversalReflectBxDFSet::evaluate_local(const Float3 &wo, const Float3 &wi,
                                                    vision::MaterialEvalMode mode,
                                                    const Uint &flag) const noexcept {
    return refl_->safe_evaluate(wo, wi, fresnel_->clone(), mode);
}

BSDFSample UniversalReflectBxDFSet::sample_local(const Float3 &wo, const Uint &flag,
                                                 vision::TSampler &sampler) const noexcept {
    return refl_->sample(wo, sampler, fresnel_->clone());
}

BSDFSample UniversalReflectBxDFSet::sample_delta_local(const Float3 &wo,
                                                       TSampler &sampler) const noexcept {
    Float3 wi = make_float3(-wo.xy(), wo.z);
    BSDFSample ret{refl_->swl()};
    ret.wi = wi;
    ret.eval = refl_->evaluate(wo, wi, fresnel_->clone(), All);
    return ret;
}

SampledDirection UniversalReflectBxDFSet::sample_wi(const Float3 &wo,
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

}// namespace vision