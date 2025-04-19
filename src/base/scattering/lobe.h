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

struct Lobe : public ocarina::Hashable {
public:
    static constexpr float alpha_lower = 0.001f;
    static constexpr float alpha_upper = 1.f;

public:
    Lobe() = default;
    [[nodiscard]] virtual SampledSpectrum albedo(const Float &cos_theta) const noexcept = 0;
    [[nodiscard]] virtual ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode, const Uint &flag,
                                                     TransportMode tm) const noexcept = 0;
    [[nodiscard]] virtual ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                                     const Uint &flag, TransportMode tm, Float *eta) const noexcept {
        return evaluate_local(wo, wi, mode, flag, tm);
    }
    [[nodiscard]] virtual Float valid_factor(const Float3 &wo, const Float3 &wi) const noexcept;
    [[nodiscard]] virtual BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                                  TSampler &sampler,
                                                  TransportMode tm) const noexcept;
    [[nodiscard]] virtual SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                                     TSampler &sampler) const noexcept {
        OC_ASSERT(false);
        return {};
    }
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
    MicrofacetLobe(const SP<Fresnel> &fresnel, UP<MicrofacetBxDF> refl);
    template<typename T = Fresnel>
    [[nodiscard]] const T *fresnel() const noexcept { return static_cast<const T *>(fresnel_.get()); }
    template<typename T = Fresnel>
    [[nodiscard]] T *fresnel() noexcept { return static_cast<T *>(fresnel_.get()); }
    [[nodiscard]] const MicrofacetBxDF *bxdf() const noexcept { return bxdf_.get(); }
    [[nodiscard]] MicrofacetBxDF *bxdf() noexcept { return bxdf_.get(); }
    void from_ratio_x(const ocarina::Float &roughness) noexcept override;
    [[nodiscard]] Float to_ratio_x() const noexcept override;
    VS_MAKE_LOBE_ASSIGNMENT(MicrofacetLobe)
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag,
                                             TransportMode tm) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler,
                                          TransportMode tm) const noexcept override;
    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo,
                                                TSampler &sampler) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::GlossyRefl; }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
};

class DiffuseLobe : public Lobe {
private:
    DCUP<BxDF> bxdf_;

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return bxdf_->topology_hash();
    }

public:
    DiffuseLobe(const SampledSpectrum &kr, const SampledWavelengths &swl)
        : bxdf_(std::make_unique<LambertReflection>(kr, swl)) {}
    DiffuseLobe(SampledSpectrum R, Float sigma, const SampledWavelengths &swl)
        : bxdf_(std::make_unique<OrenNayar>(R, sigma, swl)) {}
    [[nodiscard]] Uint flag() const noexcept override { return bxdf_->flags(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override { return bxdf_->albedo(cos_theta); }
    [[nodiscard]] const SampledWavelengths *swl() const override {
        return &bxdf_->swl();
    }
    // clang-format off
    VS_MAKE_LOBE_ASSIGNMENT(DiffuseLobe)
    // clang-format on

    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode,
                                             const Uint &flag,
                                             TransportMode tm) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler,
                                          TransportMode tm) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
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

public:
    DielectricLobe(const SP<Fresnel> &fresnel, const SP<Microfacet<D>> &microfacet,
                   SampledSpectrum color, Bool dispersive, Uint flag)
        : fresnel_(fresnel), microfacet_(microfacet),
          kt_(std::move(color)), dispersive_(ocarina::move(dispersive)),
          flag_(std::move(flag)) {}
    VS_MAKE_LOBE_ASSIGNMENT(DielectricLobe)
    [[nodiscard]] virtual bool compensate() const noexcept { return true; }

    static void prepare() noexcept;
    [[nodiscard]] Float valid_factor(const Float3 &wo, const Float3 &wi) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override { return fresnel_->swl(); }
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override { return dispersive_; }
    [[nodiscard]] Bool splittable() const noexcept override { return true; }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm, Float *eta) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override { return flag_; }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                          TransportMode tm) const noexcept override;
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

public:
    LobeSet() = default;
    explicit LobeSet(Lobes lobes) : lobes_(std::move(lobes)) {
        normalize_sampled_weights();
    }
    void normalize_sampled_weights() noexcept;
    VS_MAKE_LOBE_ASSIGNMENT(LobeSet)
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] uint lobe_num() const noexcept { return lobes_.size(); }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm) const noexcept override;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm, Float *eta) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler, TransportMode tm) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override { return lobes_[0]->swl(); }
    void for_each(const std::function<void(const WeightedLobe &)> &func) const;
    void for_each(const std::function<void(WeightedLobe &)> &func);
    void for_each(const std::function<void(const WeightedLobe &, uint)> &func) const;
    void for_each(const std::function<void(WeightedLobe &, uint)> &func);
};

class MixLobe : public LobeSet {
public:
    using LobeSet::LobeSet;
    MixLobe() = default;
    MixLobe(const Float &frac, SP<Lobe> b0, SP<Lobe> b1) {
        Float w0 = 1 - frac;
        Float w1 = frac;
        WeightedLobe wb0{w0, w0, std::move(b0)};
        WeightedLobe wb1{w1, w1, std::move(b1)};
        lobes_.push_back(std::move(wb0));
        lobes_.push_back(std::move(wb1));
        OC_ERROR_IF(lobes_.size() != 2, "MixLobe lobe num must be 2");
    }
};

class AddLobe : public LobeSet {
public:
    using LobeSet::LobeSet;
    AddLobe() = default;
    AddLobe(SP<Lobe> b0, SP<Lobe> b1) {
        WeightedLobe wb0{1.f, 1.f, std::move(b0)};
        WeightedLobe wb1{1.f, 1.f, std::move(b1)};
        lobes_.push_back(std::move(wb0));
        lobes_.push_back(std::move(wb1));
        normalize_sampled_weights();
        OC_ERROR_IF(lobes_.size() != 2, "AddLobe lobe num must be 2");
    }
};

class PureReflectionLobe : public MicrofacetLobe {
public:
    using MicrofacetLobe::MicrofacetLobe;
    static constexpr const char *lut_name = "PureReflectionLobe::lut";
    static constexpr uint lut_res = 32;
    [[nodiscard]] virtual Float compensate_factor(const Float3 &wo) const noexcept;
    [[nodiscard]] virtual bool compensate() const noexcept { return false; }
    static void prepare();

    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                          TransportMode tm) const noexcept override {
        BSDFSample bs = MicrofacetLobe::sample_local(wo, flag, sampler, tm);
        if (compensate()) {
            bs.eval.f *= compensate_factor(wo);
        }
        return bs;
    }

    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag, TransportMode tm) const noexcept override {
        ScatterEval se = MicrofacetLobe::evaluate_local(wo, wi, mode, flag, tm);
        if (compensate()) {
            se.f *= compensate_factor(wo);
        }
        return se;
    }

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
