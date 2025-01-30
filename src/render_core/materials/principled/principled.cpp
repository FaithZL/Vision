//
// Created by Zero on 2025/1/16.
//

#include <utility>
#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"
#include "ltc_sheen.h"

namespace vision {

class FresnelGeneralizedSchlick : public Fresnel {
private:
    SampledSpectrum F0_;
    Float eta_;

public:
    FresnelGeneralizedSchlick(SampledSpectrum F0, Float eta,
                              const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)), eta_(std::move(eta)) {}

    void correct_eta(Float cos_theta) noexcept override {
        eta_ = select(cos_theta > 0, eta_, rcp(eta_));
    }

    [[nodiscard]] SampledSpectrum evaluate(ocarina::Float cos_theta) const noexcept override {
        Float F_real = fresnel_dielectric(cos_theta, eta_);
        Float F0_real = schlick_weight(eta_);
        Float t = inverse_lerp(F_real, F0_real, 1.f);
        t = ocarina::clamp(t, 0.f, 1.f);
        return lerp(t, F0_, 1.f);
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override {
        return {swl_->dimension(), eta_};
    }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelGeneralizedSchlick>(F0_, eta_, *swl_);
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelGeneralizedSchlick)
};

class FresnelF82Tint : public Fresnel {
private:
    SampledSpectrum F0_;
    SampledSpectrum B_;

public:
    using Fresnel::Fresnel;

    FresnelF82Tint(SampledSpectrum F0, SampledSpectrum B,
                   const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)), B_(std::move(B)) {
    }

    FresnelF82Tint(SampledSpectrum F0, const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)), B_(SampledSpectrum::one(swl.dimension())) {}

    void init_from_F82(SampledSpectrum F82) {
        static constexpr float f = 6.f / 7.f;
        static constexpr float f5 = Pow<5>(f);
        SampledSpectrum one = SampledSpectrum::one(swl_->dimension());
        SampledSpectrum f_schlick = lerp(f5, F0_, one);
        B_ = f_schlick * (7.f / (f5 * f)) * (one - F82);
    }

    [[nodiscard]] SampledSpectrum evaluate(ocarina::Float cos_theta) const noexcept override {
        Float mu = ocarina::saturate(1.f - cos_theta);
        Float mu5 = Pow<5>(mu);
        SampledSpectrum f_schlick = lerp(mu5, F0_, SampledSpectrum::one(swl_->dimension()));
        SampledSpectrum ret = saturate(f_schlick - B_ * cos_theta * mu5 * mu);
        return ret;
    }

    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelF82Tint>(F0_, B_, *swl_);
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelF82Tint)
};

class WeightedBxDFSet {
private:
    Float weight_;
    DCSP<BxDFSet> bxdf_;

public:
    WeightedBxDFSet(Float weight, SP<BxDFSet> bxdf)
        : weight_(std::move(weight)), bxdf_(std::move(bxdf)) {}
    [[nodiscard]] const BxDFSet &operator*() const noexcept { return *bxdf_; }
    [[nodiscard]] BxDFSet &operator*() noexcept { return *bxdf_; }
    [[nodiscard]] const BxDFSet *operator->() const noexcept { return bxdf_.get(); }
    [[nodiscard]] BxDFSet *operator->() noexcept { return bxdf_.get(); }
    [[nodiscard]] const BxDFSet *get() const noexcept { return bxdf_.get(); }
    [[nodiscard]] BxDFSet *get() noexcept { return bxdf_.get(); }
    OC_MAKE_MEMBER_GETTER(weight, &)
};

class MultiBxDFSet : public BxDFSet {
public:
    using Lobes = ocarina::vector<WeightedBxDFSet>;

private:
    Lobes lobes_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        uint64_t ret = Hash64::default_seed;
        for_each([&](const WeightedBxDFSet &lobe) {
            ret = hash64(ret, lobe->type_hash());
        });
        return ret;
    }

public:
    MultiBxDFSet() = default;
    explicit MultiBxDFSet(Lobes lobes) : lobes_(std::move(lobes)) {
        normalize_weights();
    }
    void normalize_weights() noexcept;
    VS_MAKE_BxDFSet_ASSIGNMENT(MultiBxDFSet)
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override;
    [[nodiscard]] uint lobe_num() const noexcept { return lobes_.size(); }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override { return lobes_[0]->swl(); }
    void for_each(const std::function<void(const WeightedBxDFSet &)> &func) const;
    void for_each(const std::function<void(WeightedBxDFSet &)> &func);
    void for_each(const std::function<void(const WeightedBxDFSet &, uint)> &func) const;
    void for_each(const std::function<void(WeightedBxDFSet &, uint)> &func);
};

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

SampledSpectrum MultiBxDFSet::albedo(const ocarina::Float3 &wo) const noexcept {
    SampledSpectrum ret = SampledSpectrum::zero(swl()->dimension());
    for_each([&](const WeightedBxDFSet &lobe) {
        ret += lobe->albedo(wo) * lobe.weight();
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
        $condition_info("{} -----", lobe.weight());
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
    $condition_info(" {} ", sampling_strategy);
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

class PrincipledMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(metallic)
    VS_MAKE_SLOT(ior)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(spec_tint)
    VS_MAKE_SLOT(anisotropic)

    VS_MAKE_SLOT(sheen_weight)
    VS_MAKE_SLOT(sheen_roughness)
    VS_MAKE_SLOT(sheen_tint)

    VS_MAKE_SLOT(clearcoat_weight)
    VS_MAKE_SLOT(clearcoat_roughness)
    VS_MAKE_SLOT(clearcoat_tint)

    VS_MAKE_SLOT(subsurface_weight)
    VS_MAKE_SLOT(subsurface_radius)
    VS_MAKE_SLOT(subsurface_scale)

    VS_MAKE_SLOT(spec_trans)

protected:
    VS_MAKE_MATERIAL_EVALUATOR(MultiBxDFSet)
    static constexpr float CutoffThreshold = 1e-5f;

public:
    PrincipledMaterial() = default;
    explicit PrincipledMaterial(const MaterialDesc &desc)
        : Material(desc) {

#define INIT_SLOT(name, default_value, type) \
    name##_.set(Slot::create_slot(desc.slot(#name, default_value, type)));

        INIT_SLOT(color, make_float3(1.f), Albedo);
        INIT_SLOT(metallic, 0.f, Number);
        INIT_SLOT(ior, 1.5f, Number);
        INIT_SLOT(roughness, 0.5f, Number);
        INIT_SLOT(spec_tint, make_float3(0.f), Albedo);
        INIT_SLOT(anisotropic, 0.f, Number);

        INIT_SLOT(sheen_weight, 0.f, Number);
        INIT_SLOT(sheen_roughness, 0.5f, Number);
        INIT_SLOT(sheen_tint, make_float3(1.f), Albedo);

        INIT_SLOT(clearcoat_weight, 0.3f, Number);
        INIT_SLOT(clearcoat_roughness, 0.2f, Number);
        INIT_SLOT(clearcoat_tint, make_float3(1.f), Albedo);

        INIT_SLOT(subsurface_weight, 0.3f, Number);
        INIT_SLOT(subsurface_radius, make_float3(1.f), Number);
        INIT_SLOT(subsurface_scale, 0.2f, Number);

        INIT_SLOT(spec_trans, 0.f, Number);

#undef INIT_SLOT
        init_slot_cursor(&color_, &spec_trans_);
    }
    void restore(RuntimeObject *old_obj) noexcept override {
        Material::restore(old_obj);
    }
    void prepare() noexcept override {
        SheenLTCTable::instance().init();
    }
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        MultiBxDFSet::Lobes lobes;
        auto [color, color_lum] = color_.eval_albedo_spectrum(it, swl);
        Float metallic = metallic_.evaluate(it, swl).as_scalar();
        Float ior = ior_.evaluate(it, swl).as_scalar();
        Float roughness = ocarina::clamp(roughness_.evaluate(it, swl).as_scalar(), 0.0001f, 1.f);
        Float anisotropic = anisotropic_.evaluate(it, swl).as_scalar();
        SampledSpectrum specular_tint = spec_tint_.eval_albedo_spectrum(it, swl).sample;

        SampledSpectrum sheen_tint = sheen_tint_.eval_albedo_spectrum(it, swl).sample;
        Float sheen_weight = sheen_weight_.evaluate(it, swl).as_scalar();
        Float sheen_roughness = sheen_roughness_.evaluate(it, swl).as_scalar();

        Float aspect = sqrt(1 - anisotropic * 0.9f);
        Float2 alpha = make_float2(max(0.001f, sqr(roughness) / aspect),
                                   max(0.001f, sqr(roughness) * aspect));
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);

        SampledSpectrum weight = SampledSpectrum::one(swl.dimension());

        // sheen
        Float cos_theta = dot(it.wo, it.ng);
        UP<SheenLTC> sheen_lobe = make_unique<SheenLTC>(cos_theta, sheen_tint, sheen_roughness, swl);


        // metallic
        SP<FresnelF82Tint> fresnel_f82 = make_shared<FresnelF82Tint>(color, swl);
        fresnel_f82->init_from_F82(specular_tint);
        UP<BxDF> metal_refl = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl.dimension()) * metallic, swl, microfacet);
        WeightedBxDFSet metal_lobe(metallic, make_unique<UniversalReflectBxDFSet>(fresnel_f82, std::move(metal_refl)));
        lobes.push_back(std::move(metal_lobe));

        // specular
        Float f0 = schlick_F0_from_eta(ior);
        Float spec_weight = 1 - metallic;
        SP<FresnelGeneralizedSchlick> fresnel_schlick = make_shared<FresnelGeneralizedSchlick>(f0 * specular_tint, ior, swl);
        UP<BxDF> spec_refl = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl.dimension()) * spec_weight, swl, microfacet);
        WeightedBxDFSet specular_lobe(1 - metallic, make_unique<UniversalReflectBxDFSet>(fresnel_schlick, std::move(spec_refl)));
        lobes.push_back(std::move(specular_lobe));

        // diffuse
        Float diff_weight = 1 - metallic;
        WeightedBxDFSet diffuse_lobe{diff_weight, make_shared<DiffuseBxDFSet>(color * diff_weight, swl)};
        lobes.push_back(std::move(diffuse_lobe));

        return make_unique<MultiBxDFSet>(std::move(lobes));
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PrincipledMaterial)
VS_REGISTER_CURRENT_PATH(0, "vision-material-principled.dll")