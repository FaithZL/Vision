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

}// namespace vision