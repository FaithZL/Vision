//
// Created by ling.zhu on 2025/1/25.
//

#pragma once

#include "bxdf.h"
#include "interaction.h"

namespace vision {

#define VS_MAKE_LOBE_ASSIGNMENT(ClassName)                            \
    ClassName &operator=(const Lobe &other) noexcept override {       \
        OC_ASSERT(dynamic_cast<const ClassName *>(&other));           \
        *this = dynamic_cast<ClassName &>(const_cast<Lobe &>(other)); \
        return *this;                                                 \
    }

class LobeSet;

struct Lobe : public ocarina::Hashable {
public:
    static constexpr float alpha_lower = 0.001f;
    static constexpr float alpha_upper = 1.f;

protected:
    const LobeSet *parent_{nullptr};
    optional<PartialDerivative<Float3>> shading_frame_{};

protected:
    [[nodiscard]] virtual SampledDirection sample_wi_local_impl(const Float3 &wo, const Uint &flag,
                                                                TSampler &sampler) const noexcept {
        OC_ASSERT(false);
        return {};
    }
    [[nodiscard]] virtual SampledDirection sample_wi_impl(const Float3 &world_wo, const Uint &flag,
                                                          TSampler &sampler) const noexcept;
    [[nodiscard]] virtual ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode, const Uint &flag,
                                                          TransportMode tm) const noexcept = 0;
    [[nodiscard]] virtual ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                          const Uint &flag, TransportMode tm, Float *eta) const noexcept {
        return evaluate_local_impl(wo, wi, mode, flag, tm);
    }

    [[nodiscard]] virtual ScatterEval evaluate_impl(const Float3 &world_wo, const Float3 &world_wi, MaterialEvalMode mode,
                                                    const Uint &flag, TransportMode tm) const noexcept;
    [[nodiscard]] virtual ScatterEval evaluate_impl(const Float3 &world_wo, const Float3 &world_wi, MaterialEvalMode mode,
                                                    const Uint &flag, TransportMode tm, Float *eta) const noexcept;

public:
    Lobe() = default;
    Lobe(optional<PartialDerivative<Float3>> shading_frame) : shading_frame_(std::move(shading_frame)) {}
    [[nodiscard]] virtual SampledSpectrum albedo(const Float &cos_theta) const noexcept = 0;
    [[nodiscard]] virtual const PartialDerivative<Float3> &shading_frame() const noexcept;
    virtual void set_shading_frame(const PartialDerivative<Float3> &frame) noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(parent, &)
    [[nodiscard]] ScatterEval evaluate(const Float3 &world_wo, const Float3 &world_wi, MaterialEvalMode mode,
                                       const Uint &flag, TransportMode tm) const noexcept;
    [[nodiscard]] ScatterEval evaluate(const Float3 &world_wo, const Float3 &world_wi, MaterialEvalMode mode,
                                       const Uint &flag, TransportMode tm, Float *eta) const noexcept;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm) const noexcept;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm, Float *eta) const noexcept;
    [[nodiscard]] virtual bool is_multi() const noexcept { return false; }
    [[nodiscard]] virtual Float valid_world_factor(const Float3 &wo, const Float3 &wi) const noexcept;
    [[nodiscard]] virtual Float valid_factor(const Float3 &wo, const Float3 &wi) const noexcept;
    [[nodiscard]] BSDFSample sample(const Float3 &world_wo, const Uint &flag,
                                    TSampler &sampler, TransportMode tm) const noexcept;
    [[nodiscard]] virtual BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                                  TSampler &sampler,
                                                  TransportMode tm) const noexcept;
    [[nodiscard]] SampledDirection sample_wi_local(const Float3 &wo, const Uint &flag,
                                                   TSampler &sampler) const noexcept;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &world_wo, const Uint &flag,
                                             TSampler &sampler) const noexcept;
    [[nodiscard]] virtual BSDFSample sample_delta_local(const Float3 &wo,
                                                        TSampler &sampler) const noexcept {
        return BSDFSample{1u, 1u};
    }
    [[nodiscard]] virtual Bool splittable() const noexcept { return false; }
    virtual Lobe &operator=(const Lobe &other) noexcept = default;
    virtual void regularize() noexcept {}
    virtual void mollify() noexcept {}
    [[nodiscard]] virtual const SampledWavelengths *swl() const = 0;
    [[nodiscard]] virtual Uint flag() const noexcept = 0;
    /// for precompute the table begin
    [[nodiscard]] virtual SampledSpectrum integral_albedo(const Float3 &wo, TSampler &sampler,
                                                          const Uint &sample_num) const noexcept;
    virtual SampledSpectrum precompute_with_radio(const Float3 &ratio, TSampler &sampler,
                                                  const Uint &sample_num) noexcept;
    virtual void from_ratio_x(const Float &x) noexcept;
    [[nodiscard]] static Float3 from_ratio_y(Float cos_theta) noexcept;
    virtual void from_ratio_z(Float z) noexcept;
    /// for precompute the table end

    /// for look up the table begin
    [[nodiscard]] virtual Float to_ratio_x() const noexcept;
    [[nodiscard]] static Float to_ratio_y(const Float3 &wo) noexcept;
    [[nodiscard]] virtual Float to_ratio_z() const noexcept;
    /// for look up the table end

    [[nodiscard]] virtual optional<Bool> is_dispersive() const noexcept { return {}; }
    virtual ~Lobe() = default;
};

class MicrofacetLobe : public Lobe {
protected:
    DCSP<Fresnel> fresnel_;
    DCUP<MicrofacetBxDF> bxdf_;

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override;

public:
    MicrofacetLobe(const SP<Fresnel> &fresnel, UP<MicrofacetBxDF> refl,
                   optional<PartialDerivative<Float3>> shading_frame = {});

    template<typename T = Fresnel>
    [[nodiscard]] const T *fresnel() const noexcept { return static_cast<const T *>(fresnel_.get()); }
    template<typename T = Fresnel>
    [[nodiscard]] T *fresnel() noexcept { return static_cast<T *>(fresnel_.get()); }
    [[nodiscard]] const MicrofacetBxDF *bxdf() const noexcept { return bxdf_.get(); }
    [[nodiscard]] MicrofacetBxDF *bxdf() noexcept { return bxdf_.get(); }
    [[nodiscard]] Microfacet<D> *microfacet() noexcept { return bxdf()->microfacet(); }
    [[nodiscard]] const Microfacet<D> *microfacet() const noexcept { return bxdf()->microfacet(); }
    void from_ratio_x(const ocarina::Float &roughness) noexcept override;
    [[nodiscard]] Float to_ratio_x() const noexcept override;
    VS_MAKE_LOBE_ASSIGNMENT(MicrofacetLobe)
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override;
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                                  MaterialEvalMode mode, const Uint &flag,
                                                  TransportMode tm) const noexcept override;
    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo,
                                                TSampler &sampler) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::GlossyRefl; }
    [[nodiscard]] SampledDirection sample_wi_local_impl(const Float3 &wo, const Uint &flag,
                                                        TSampler &sampler) const noexcept override;
};

class DiffuseLobe : public Lobe {
private:
    DCUP<BxDF> bxdf_;

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return bxdf_->topology_hash();
    }
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                                  MaterialEvalMode mode,
                                                  const Uint &flag,
                                                  TransportMode tm) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi_local_impl(const Float3 &wo, const Uint &flag,
                                                        TSampler &sampler) const noexcept override;

public:
    DiffuseLobe(const SampledSpectrum &kr, const SampledWavelengths &swl,
                optional<PartialDerivative<Float3>> shading_frame = {})
        : Lobe(std::move(shading_frame)), bxdf_(std::make_unique<LambertReflection>(kr, swl)) {}
    DiffuseLobe(SampledSpectrum R, Float sigma, const SampledWavelengths &swl,
                optional<PartialDerivative<Float3>> shading_frame = {})
        : Lobe(std::move(shading_frame)), bxdf_(std::make_unique<OrenNayar>(R, sigma, swl)) {}
    [[nodiscard]] Uint flag() const noexcept override { return bxdf_->flags(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override { return bxdf_->albedo(cos_theta); }
    [[nodiscard]] const SampledWavelengths *swl() const override {
        return &bxdf_->swl();
    }
    VS_MAKE_LOBE_ASSIGNMENT(DiffuseLobe)
};

[[nodiscard]] inline Float ior_to_ratio_z(const Float &ior) {
    return ocarina::sqrt(ocarina::abs((ior - 1.0f) / (ior + 1.0f)));
}

[[nodiscard]] inline Float ior_from_ratio_z(const Float &z) {
    Float ior = schlick_ior_from_F0(Pow<4>(ocarina::clamp(z, 0.001f, 1.f)));
    return ior;
}

class DielectricLobe : public Lobe {
public:
    static constexpr float ior_lower = 1.003;
    static constexpr float ior_upper = 5.f;
    static constexpr const char *lut_name = "DielectricLobe::lut";
    static constexpr const char *lut_inv_name = "DielectricLobeInv::lut";
    static constexpr uint lut_res = 32;

protected:
    DCSP<Fresnel> fresnel_;
    Bool dispersive_{};
    DCSP<Microfacet<D>> microfacet_;
    SampledSpectrum kt_{};
    Uint flag_{};

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return hash64(fresnel_->topology_hash());
    }
    [[nodiscard]] static Uint select_lut(const SampledSpectrum &eta) noexcept;
    static Float eta_to_ratio_z(const Float &eta) noexcept;
    [[nodiscard]] Float2 sample_lut(const Float3 &wo, const SampledSpectrum &eta) const noexcept;
    [[nodiscard]] Float refl_compensate(const Float3 &wo, const SampledSpectrum &eta) const noexcept;
    [[nodiscard]] Float trans_compensate(const Float3 &wo, const SampledSpectrum &eta) const noexcept;
    [[nodiscard]] ScatterEval evaluate_reflection(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                  const SampledSpectrum &F, const SampledSpectrum &eta,
                                                  MaterialEvalMode mode) const noexcept;
    [[nodiscard]] ScatterEval evaluate_transmission(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                                    const SampledSpectrum &F, const SampledSpectrum &eta,
                                                    MaterialEvalMode mode,
                                                    TransportMode tm) const noexcept;
    [[nodiscard]] ScatterEval evaluate_impl(const Float3 &wo, const Float3 &wh, const Float3 &wi,
                                            const SP<const Fresnel> &fresnel, MaterialEvalMode mode,
                                            TransportMode tm) const noexcept;
    [[nodiscard]] Float refl_prob(const SampledSpectrum &F) const noexcept;
    [[nodiscard]] Float trans_prob(const SampledSpectrum &F) const noexcept;
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                  const Uint &flag, TransportMode tm) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                  const Uint &flag, TransportMode tm, Float *eta) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi_local_impl(const Float3 &wo, const Uint &flag,
                                                        TSampler &sampler) const noexcept override;

public:
    DielectricLobe(const SP<Fresnel> &fresnel, const SP<Microfacet<D>> &microfacet,
                   SampledSpectrum color, Bool dispersive, Uint flag,
                   optional<PartialDerivative<Float3>> shading_frame = {})
        : Lobe(std::move(shading_frame)), fresnel_(fresnel), microfacet_(microfacet),
          kt_(std::move(color)), dispersive_(ocarina::move(dispersive)),
          flag_(std::move(flag)) {}
    VS_MAKE_LOBE_ASSIGNMENT(DielectricLobe)
    [[nodiscard]] virtual bool compensate() const noexcept { return true; }
    static void prepare() noexcept;
    [[nodiscard]] Float valid_world_factor(const Float3 &wo, const Float3 &wi) const noexcept override;
    [[nodiscard]] Float valid_factor(const Float3 &wo, const Float3 &wi) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override { return fresnel_->swl(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override { return dispersive_; }
    [[nodiscard]] Bool splittable() const noexcept override { return true; }

    [[nodiscard]] Uint flag() const noexcept override { return flag_; }
    [[nodiscard]] Float to_ratio_z() const noexcept override {
        Float ior = fresnel_->eta().average();
        return inverse_lerp(ior, ior_lower, ior_upper);
    }
    Float to_ratio_x() const noexcept override {
        Float ax = microfacet_->alpha_x();
        Float ay = microfacet_->alpha_y();
        Float a = sqrt(ax * ay);
        return ocarina::sqrt(a);
    }
};

class WeightedLobe {
private:
    DCSP<Lobe> bxdf_;
    Float sample_weight_;
    Float weight_;

public:
    WeightedLobe(Float weight, SP<Lobe> bxdf);
    WeightedLobe(Float sample_weight, Float weight, SP<Lobe> bxdf);
    [[nodiscard]] const Lobe &operator*() const noexcept { return *bxdf_; }
    [[nodiscard]] Lobe &operator*() noexcept { return *bxdf_; }
    [[nodiscard]] const Lobe *operator->() const noexcept { return bxdf_.get(); }
    [[nodiscard]] Lobe *operator->() noexcept { return bxdf_.get(); }
    [[nodiscard]] const Lobe *get() const noexcept { return bxdf_.get(); }
    [[nodiscard]] Lobe *get() noexcept { return bxdf_.get(); }
    OC_MAKE_MEMBER_GETTER(sample_weight, &)
    OC_MAKE_MEMBER_GETTER(weight, &)
};

class LobeSet : public Lobe {
public:
    using Lobes = ocarina::vector<WeightedLobe>;

protected:
    Lobes lobes_;

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        uint64_t ret = Hash64::default_seed;
        for_each([&](const WeightedLobe &lobe) {
            ret = hash64(ret, lobe->topology_hash());
        });
        return ret;
    }
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                  const Uint &flag, TransportMode tm) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                  const Uint &flag, TransportMode tm, Float *eta) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi_local_impl(const Float3 &wo, const Uint &flag,
                                                        TSampler &sampler) const noexcept override;
    SampledDirection sample_wi_impl(const Float3 &world_wo, const Uint &flag,
                                    TSampler &sampler) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_impl(const Float3 &world_wo, const Float3 &world_wi, MaterialEvalMode mode,
                                            const Uint &flag, TransportMode tm) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_impl(const Float3 &world_wo, const Float3 &world_wi, MaterialEvalMode mode,
                                            const Uint &flag, TransportMode tm, Float *eta) const noexcept override;

public:
    LobeSet() = default;
    explicit LobeSet(Lobes lobes) : lobes_(std::move(lobes)) {
        initialize();
    }
    /// normalize weight and flatten the tree
    void initialize() noexcept;
    void update_children_parents() noexcept;
    static UP<LobeSet> create_mix(const Float &frac, SP<Lobe> b0, SP<Lobe> b1) noexcept;
    static UP<LobeSet> create_add(SP<Lobe> b0, SP<Lobe> b1) noexcept;
    void normalize_sampled_weight() noexcept;
    void flatten() noexcept;
    [[nodiscard]] bool is_multi() const noexcept override { return true; }
    VS_MAKE_LOBE_ASSIGNMENT(LobeSet)
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] uint lobe_num() const noexcept { return lobes_.size(); }
    [[nodiscard]] Uint flag() const noexcept override;
    [[nodiscard]] Float valid_world_factor(const Float3 &wo, const Float3 &wi) const noexcept override;
    [[nodiscard]] Float valid_factor(const Float3 &wo, const Float3 &wi) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override { return lobes_[0]->swl(); }
    void for_each(const std::function<void(const WeightedLobe &)> &func) const;
    void for_each(const std::function<void(WeightedLobe &)> &func);
    void for_each(const std::function<void(const WeightedLobe &, uint)> &func) const;
    void for_each(const std::function<void(WeightedLobe &, uint)> &func);
};

class PureReflectionLobe : public MicrofacetLobe {
protected:
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                  const Uint &flag, TransportMode tm) const noexcept override;

public:
    using MicrofacetLobe::MicrofacetLobe;
    static constexpr const char *lut_name = "PureReflectionLobe::lut";
    static constexpr uint lut_res = 32;
    [[nodiscard]] virtual Float compensate_factor(const Float3 &wo) const noexcept;
    [[nodiscard]] virtual bool compensate() const noexcept { return false; }
    static void prepare();

    /// for precompute begin
    static constexpr const char *name = "PureReflectionLobe";
    static UP<PureReflectionLobe> create_for_precompute(const SampledWavelengths &swl) noexcept {
        SP<Fresnel> fresnel = make_shared<FresnelConstant>(swl);
        SP<GGXMicrofacet> microfacet = make_shared<GGXMicrofacet>(0.00f, 0.0f, true);
        UP<MicrofacetBxDF> bxdf = make_unique<MicrofacetReflection>(SampledSpectrum::one(swl), swl, microfacet);
        auto ret = make_unique<PureReflectionLobe>(fresnel, std::move(bxdf));
        return ret;
    }
    void from_ratio_z(ocarina::Float z) noexcept override {
        // empty
    }
    /// for precompute end
};

class MetallicLobe : public PureReflectionLobe {
public:
    using PureReflectionLobe::PureReflectionLobe;
    bool compensate() const noexcept override { return true; }
};

}// namespace vision
