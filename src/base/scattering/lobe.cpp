//
// Created by ling.zhu on 2025/1/25.
//

#include "lobe.h"
#include "material.h"
#include "base/mgr/registries.h"
#include "precomputed_table.h"

namespace vision {

///#region Lobe
SampledSpectrum Lobe::integral_albedo(const Float3 &wo, TSampler &sampler,
                                      const Uint &sample_num) const noexcept {
    SampledSpectrum ret = SampledSpectrum::zero(*swl());
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

SampledSpectrum Lobe::precompute_with_radio(const Float3 &ratio, TSampler &sampler,
                                            const Uint &sample_num) noexcept {
    from_ratio_x(ratio.x);
    Float3 wo = from_ratio_y(ratio.y);
    from_ratio_z(ratio.z);
    return integral_albedo(wo, sampler, sample_num);
}

Float Lobe::valid_factor(const Float3 &wo, const Float3 &wi) const noexcept {
    Bool valid = same_hemisphere(wo, wi);
    return cast<float>(valid);
}

ScatterEval Lobe::evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                 const Uint &flag, TransportMode tm) const noexcept {
    string label = string(class_name()) + "::evaluate_local";
    return outline(label, [&] {
        return evaluate_local_impl(wo, wi, mode, flag, tm);
    });
}

ScatterEval Lobe::evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                 const Uint &flag, TransportMode tm, Float *eta) const noexcept {
    string label = string(class_name()) + "::evaluate_local with eta";
    return outline(label, [&] {
        return evaluate_local_impl(wo, wi, mode, flag, tm, eta);
    });
}

SampledDirection Lobe::sample_wi(const Float3 &wo, const Uint &flag,
                                 TSampler &sampler) const noexcept {
    string label = string(class_name()) + "::sample_wi";
    return outline(label, [&] {
        return sample_wi_impl(wo, flag, sampler);
    });
}

BSDFSample Lobe::sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                              TransportMode tm) const noexcept {
    BSDFSample ret{*swl()};
    SampledDirection sd = sample_wi(wo, flag, sampler);
    ret.wi = sd.wi;
    ret.eval = evaluate_local(wo, sd.wi, MaterialEvalMode::All, flag, tm, addressof(ret.eta));
    ret.eval.pdfs *= sd.factor();
    return ret;
}

void Lobe::from_ratio_x(const Float &x) noexcept {
    OC_ASSERT(0);
}

Float Lobe::to_ratio_x() const noexcept {
    OC_ASSERT(0);
    return 0;
}

Float3 Lobe::from_ratio_y(Float cos_theta) noexcept {
    cos_theta = ocarina::clamp(cos_theta, 1e-4f, 1.0f);
    Float z = cos_theta;
    Float y = 0;
    Float x = ocarina::sqrt(1 - sqr(z));
    return make_float3(x, y, z);
}

Float Lobe::to_ratio_y(const ocarina::Float3 &wo) noexcept {
    return abs_cos_theta(wo);
}

void Lobe::from_ratio_z(ocarina::Float z) noexcept {
    OC_ASSERT(0);
}

Float Lobe::to_ratio_z() const noexcept {
    OC_ASSERT(0);
    return 0;
}
///#endregion

///#region MicrofacetLobe
uint64_t MicrofacetLobe::compute_topology_hash() const noexcept {
    return hash64(fresnel_->topology_hash(), bxdf_->topology_hash());
}

MicrofacetLobe::MicrofacetLobe(const SP<Fresnel> &fresnel,
                               UP<MicrofacetBxDF> refl)
    : fresnel_(fresnel), bxdf_(std::move(refl)) {}

void MicrofacetLobe::from_ratio_x(const Float &roughness) noexcept {
    bxdf()->set_alpha(clamp(sqr(roughness), alpha_lower, alpha_upper));
}

Float MicrofacetLobe::to_ratio_x() const noexcept {
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

const SampledWavelengths *MicrofacetLobe::swl() const {
    return &bxdf_->swl();
}

SampledSpectrum MicrofacetLobe::albedo(const Float &cos_theta) const noexcept {
    return bxdf_->albedo(cos_theta) * fresnel_->evaluate(cos_theta);
}

ScatterEval MicrofacetLobe::evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                                vision::MaterialEvalMode mode,
                                                const Uint &flag,
                                                TransportMode tm) const noexcept {
    return bxdf_->safe_evaluate(wo, wi, fresnel_.ptr(), mode, tm);
}

BSDFSample MicrofacetLobe::sample_delta_local(const Float3 &wo,
                                              TSampler &sampler) const noexcept {
    Float3 wi = make_float3(-wo.xy(), wo.z);
    BSDFSample ret{bxdf_->swl()};
    ret.wi = wi;
    ret.eval = bxdf_->evaluate(wo, wi, fresnel_.ptr(), All, Radiance);
    return ret;
}

SampledDirection MicrofacetLobe::sample_wi_impl(const Float3 &wo,
                                                const Uint &flag,
                                                TSampler &sampler) const noexcept {
    return bxdf_->sample_wi(wo, sampler->next_2d(), fresnel_.ptr());
}
///#endregion

///#region DiffuseLobe
ScatterEval DiffuseLobe::evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag,
                                             TransportMode tm) const noexcept {
    return bxdf_->safe_evaluate(wo, wi, nullptr, mode, tm);
}

SampledDirection DiffuseLobe::sample_wi_impl(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept {
    return bxdf_->sample_wi(wo, sampler->next_2d(), nullptr);
}
///#endregion

///#region DielectricLobe
void DielectricLobe::prepare() noexcept {
    MaterialLut::instance().load_lut(lut_name, make_uint3(lut_res),
                                     PixelStorage::FLOAT2,
                                     addressof(DielectricLobe_Table));

    MaterialLut::instance().load_lut(lut_inv_name, make_uint3(lut_res),
                                     PixelStorage::FLOAT2,
                                     addressof(DielectricInvLobe_Table));
}

Uint DielectricLobe::select_lut(const vision::SampledSpectrum &eta) noexcept {
    Uint idx = MaterialLut::instance().get_index(lut_name).hv();
    Uint inv_idx = MaterialLut::instance().get_index(lut_inv_name).hv();
    Uint index = ocarina::select(eta[0] > 1, idx, inv_idx);
    return index;
}

Float DielectricLobe::eta_to_ratio_z(const Float &eta) noexcept {
    Float ret = ocarina::select(eta > 1.f, inverse_lerp(eta, ior_lower, ior_upper),
                                inverse_lerp(rcp(eta), ior_lower, ior_upper));
    return ret;
}

Float2 DielectricLobe::sample_lut(const Float3 &wo, const SampledSpectrum &eta) const noexcept {
    Uint idx = select_lut(eta);
    const BindlessArray &ba = Global::instance().bindless_array();
    Float x = to_ratio_x();
    Float y = abs_cos_theta(wo);
    Float z = eta_to_ratio_z(eta[0]);
    Float3 uvw = make_float3(x, y, z);
    Float2 ret = ba.tex_var(idx).sample(2, uvw).as_vec2();
    return ret;
}

Float DielectricLobe::refl_compensate(const Float3 &wo,
                                      const SampledSpectrum &eta) const noexcept {
    if (!compensate()) {
        return 1;
    }
    Float2 val = sample_lut(wo, eta);
    Float factor = val.x;
    Float refl = val.y;
    return rcp(factor);
}

Float DielectricLobe::trans_compensate(const ocarina::Float3 &wo,
                                       const SampledSpectrum &eta) const noexcept {
    if (!compensate()) {
        return 1;
    }
    Float2 val = sample_lut(wo, eta);
    Float factor = val.x;
    return rcp(factor);
}

SampledSpectrum DielectricLobe::albedo(const Float &cos_theta) const noexcept {
    SP<Fresnel> fresnel = fresnel_.ptr();
    SampledSpectrum eta = fresnel->eta();
    SampledSpectrum F = fresnel->evaluate(abs(cos_theta));
    return kt_ * (1 - F) + F;
}

Float DielectricLobe::refl_prob(const vision::SampledSpectrum &F) const noexcept {
    SampledSpectrum T = 1 - F;
    SampledSpectrum total = T * kt_ + F;
    return F.average() / total.average();
}

Float DielectricLobe::trans_prob(const vision::SampledSpectrum &F) const noexcept {
    return 1 - refl_prob(F);
}

ScatterEval DielectricLobe::evaluate_impl(const Float3 &wo, const Float3 &wh, const Float3 &wi,
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

ScatterEval DielectricLobe::evaluate_reflection(const Float3 &wo, const Float3 &wh, const Float3 &wi,
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

ScatterEval DielectricLobe::evaluate_transmission(const Float3 &wo, const Float3 &wh, const Float3 &wi,
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

Float DielectricLobe::valid_factor(const Float3 &wo, const Float3 &wi) const noexcept {
    return 1.f;
}

ScatterEval DielectricLobe::evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                                MaterialEvalMode mode, const Uint &flag,
                                                TransportMode tm) const noexcept {
    SP<Fresnel> fresnel = fresnel_.ptr();
    Bool reflect = same_hemisphere(wo, wi);
    Float eta_p = ocarina::select(reflect, 1.f, fresnel->eta()[0]);
    Float3 wh = normalize(wo + eta_p * wi);
    return evaluate_impl(wo, wh, wi, fresnel, mode, tm);
}

ScatterEval DielectricLobe::evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                const Uint &flag, TransportMode tm, Float *eta) const noexcept {
    SP<Fresnel> fresnel = fresnel_.ptr();
    Bool reflect = same_hemisphere(wo, wi);
    Float eta_p = ocarina::select(reflect, 1.f, fresnel->eta()[0]);
    if (eta) {
        *eta = eta_p;
    }
    Float3 wh = normalize(wo + eta_p * wi);
    wh = face_forward(wh, wo);
    return evaluate_impl(wo, wh, wi, fresnel, mode, tm);
}

SampledDirection DielectricLobe::sample_wi_impl(const Float3 &wo, const Uint &flag,
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

///#endregion

///#region LobeSet
void LobeSet::for_each(const std::function<void(const WeightedLobe &)> &func) const {
    std::for_each(lobes_.begin(), lobes_.end(), func);
}

void LobeSet::for_each(const std::function<void(WeightedLobe &)> &func) {
    std::for_each(lobes_.begin(), lobes_.end(), func);
}

void LobeSet::for_each(const std::function<void(const WeightedLobe &, uint)> &func) const {
    for (int i = 0; i < lobe_num(); ++i) {
        func(lobes_[i], i);
    }
}

void LobeSet::for_each(const std::function<void(WeightedLobe &, uint)> &func) {
    for (int i = 0; i < lobe_num(); ++i) {
        func(lobes_[i], i);
    }
}

WeightedLobe::WeightedLobe(Float weight, SP<Lobe> bxdf)
    : bxdf_(bxdf), sample_weight_(std::move(weight)),
      weight_(1.f) {}

WeightedLobe::WeightedLobe(Float sample_weight, Float weight, SP<Lobe> bxdf)
    : bxdf_(bxdf), sample_weight_(std::move(sample_weight)),
      weight_(std::move(weight)) {
}

void LobeSet::initialize() noexcept {
    normalize_sampled_weight();
    flatten();
}

void LobeSet::normalize_sampled_weight() noexcept {
    Float weight_sum = 0;
    for_each([&](WeightedLobe &lobe) {
        weight_sum += lobe.sample_weight();
    });
    for_each([&](WeightedLobe &lobe) {
        lobe.sample_weight() = lobe.sample_weight() / weight_sum;
    });
}

void LobeSet::flatten() noexcept {
    if (!MaterialRegistry::instance().flatten_lobes()) {
        return;
    }
    Lobes new_lobes;
    bool has_multi = false;
    for_each([&](WeightedLobe &lobe) {
        has_multi = has_multi || lobe->is_multi();
    });
    if (!has_multi) {
        return;
    }
    comment("LobeSet::flatten start");
    for_each([&](WeightedLobe &lobe) {
        if (lobe->is_multi()) {
            LobeSet *lobe_set = static_cast<LobeSet *>(lobe.get());
            Float parent_weight = lobe.sample_weight();
            lobe_set->for_each([&](WeightedLobe &sub_lobe) {
                sub_lobe.sample_weight() *= parent_weight;
                sub_lobe.weight() *= parent_weight;
                new_lobes.push_back(sub_lobe);
            });
        } else {
            new_lobes.push_back(lobe);
        }
    });
    lobes_ = std::move(new_lobes);
    comment("LobeSet::flatten end");
}

SampledSpectrum LobeSet::albedo(const Float &cos_theta) const noexcept {
    SampledSpectrum ret = SampledSpectrum::zero(swl()->dimension());
    for_each([&](const WeightedLobe &lobe) {
        ret += lobe->albedo(cos_theta) * lobe.weight();
    });
    return ret;
}

Float LobeSet::valid_factor(const ocarina::Float3 &wo, const ocarina::Float3 &wi) const noexcept {
    if (MaterialRegistry::instance().flatten_lobes()) {
        OC_ERROR("LobeSet::valid_factor");
        return 0;
    } else {
        Bool ret = false;
        for_each([&](const WeightedLobe &lobe, uint i) {
            ret = ret | cast<bool>(lobe->valid_factor(wo, wi));
        });
        return ret;
    }
}

SampledDirection LobeSet::sample_wi_impl(const Float3 &wo, const Uint &flag,
                                         TSampler &sampler) const noexcept {
    Float uc = sampler->next_1d();
    Float2 u = sampler->next_2d();
    SampledDirection sd;
    Uint sampling_strategy = 0u;
    Float sum_weights = 0.f;

    for_each([&](const WeightedLobe &lobe, uint i) {
        sampling_strategy = select(uc > sum_weights, i, sampling_strategy);
        sum_weights += lobe.sample_weight();
    });
    if (lobe_num() == 1) {
        sd = lobes_[0]->sample_wi(wo, flag, sampler);
    } else {
        $switch(sampling_strategy) {
            for_each([&](const WeightedLobe &lobe, uint i) {
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

Uint LobeSet::flag() const noexcept {
    Uint ret = BxDFFlag::Unset;
    for_each([&](const WeightedLobe &lobe) {
        ret |= lobe->flag();
    });
    return ret;
}

ScatterEval LobeSet::evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                         MaterialEvalMode mode, const Uint &flag,
                                         TransportMode tm, Float *eta) const noexcept {
    ScatterEval ret{*swl()};
    for_each([&](const WeightedLobe &lobe) {
        ScatterEval se = lobe->evaluate_local(wo, wi, mode, flag, tm, eta);
        se.f *= lobe.weight() * lobe->valid_factor(wo, wi);
        se.pdfs *= lobe.sample_weight() * lobe->valid_factor(wo, wi);

        ret.f += se.f;
        ret.pdfs += se.pdfs;
        ret.flags = ret.flags | se.flags;
    });
    return ret;
}

ScatterEval LobeSet::evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                         MaterialEvalMode mode, const Uint &flag,
                                         TransportMode tm) const noexcept {
    return evaluate_local_impl(wo, wi, mode, flag, tm, nullptr);
}
///#endregion

///#region PureReflectionLobe
Float PureReflectionLobe::compensate_factor(const ocarina::Float3 &wo) const noexcept {
    Float alpha = bxdf()->alpha_average();
    Float ret = MaterialLut::instance().sample(lut_name, 1, make_float2(alpha, cos_theta(wo))).as_scalar();
    return 1.f / ret;
}

void PureReflectionLobe::prepare() {
    MaterialLut::instance().load_lut(lut_name, make_uint2(lut_res),
                                     PixelStorage::FLOAT1,
                                     addressof(PureReflectionLobe_Table));
}
///#endregion

}// namespace vision