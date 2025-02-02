//
// Created by ling.zhu on 2025/1/25.
//

#pragma once

#include "bxdf.h"
#include "interaction.h"

namespace vision {

#define VS_MAKE_BxDFSet_ASSIGNMENT(ClassName)                            \
    ClassName &operator=(const BxDFSet &other) noexcept override {       \
        OC_ASSERT(dynamic_cast<const ClassName *>(&other));              \
        *this = dynamic_cast<ClassName &>(const_cast<BxDFSet &>(other)); \
        return *this;                                                    \
    }

struct BxDFSet : public ocarina::Hashable {
public:
    BxDFSet() = default;
    [[nodiscard]] virtual SampledSpectrum principled_albedo(const Float &cos_theta) const noexcept = 0;
    [[nodiscard]] virtual ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                                     MaterialEvalMode mode,
                                                     const Uint &flag) const noexcept = 0;
    [[nodiscard]] virtual BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                                  TSampler &sampler) const noexcept = 0;
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
    virtual BxDFSet &operator=(const BxDFSet &other) noexcept = default;
    virtual void regularize() noexcept {}
    virtual void mollify() noexcept {}
    [[nodiscard]] virtual const SampledWavelengths *swl() const = 0;
    [[nodiscard]] virtual Uint flag() const noexcept = 0;
    /// for precompute the table begin
    [[nodiscard]] virtual SampledSpectrum precompute_albedo(const Float3 &wo, TSampler &sampler,
                                                            const Uint &sample_num) noexcept;
    virtual SampledSpectrum precompute_with_radio(const Float3 &ratio, TSampler &sampler,
                                                  const Uint &sample_num) noexcept;
    [[nodiscard]] virtual void from_ratio_x(const Float &x) noexcept;
    [[nodiscard]] static Float3 from_ratio_y(Float cos_theta) noexcept;
    [[nodiscard]] virtual void from_ratio_z(Float z) noexcept;
    /// for precompute the table end

    /// for look up the table begin
    [[nodiscard]] virtual Float to_ratio_x() noexcept;
    [[nodiscard]] static Float to_ratio_y(const Float3 &wo) noexcept;
    [[nodiscard]] virtual Float to_ratio_z() noexcept;
    /// for look up the table end

    [[nodiscard]] virtual optional<Bool> is_dispersive() const noexcept { return {}; }
    virtual ~BxDFSet() = default;
};

class MicrofacetBxDFSet : public BxDFSet {
protected:
    DCSP<Fresnel> fresnel_;
    DCUP<MicrofacetBxDF> refl_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override;

public:
    MicrofacetBxDFSet(const SP<Fresnel> &fresnel, UP<MicrofacetBxDF> refl);
    [[nodiscard]] const Fresnel *fresnel() const noexcept { return fresnel_.get(); }
    [[nodiscard]] Fresnel *fresnel() noexcept { return fresnel_.get(); }
    [[nodiscard]] const MicrofacetBxDF *bxdf() const noexcept { return refl_.get(); }
    [[nodiscard]] MicrofacetBxDF *bxdf() noexcept { return refl_.get(); }
    [[nodiscard]] Float to_ratio_x() noexcept override;
    VS_MAKE_BxDFSet_ASSIGNMENT(MicrofacetBxDFSet)
        [[nodiscard]] SampledSpectrum principled_albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override;
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo,
                                                TSampler &sampler) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::GlossyRefl; }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
};

class DiffuseBxDFSet : public BxDFSet {
private:
    DCUP<BxDF> bxdf_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return bxdf_->type_hash();
    }

public:
    DiffuseBxDFSet(const SampledSpectrum &kr, const SampledWavelengths &swl)
        : bxdf_(std::make_unique<LambertReflection>(kr, swl)) {}
    DiffuseBxDFSet(SampledSpectrum R, Float sigma, const SampledWavelengths &swl)
        : bxdf_(std::make_unique<OrenNayar>(R, sigma, swl)) {}
    [[nodiscard]] Uint flag() const noexcept override { return bxdf_->flags(); }
    [[nodiscard]] SampledSpectrum principled_albedo(const Float &cos_theta) const noexcept override { return bxdf_->principled_albedo(cos_theta); }
    [[nodiscard]] const SampledWavelengths *swl() const override {
        return &bxdf_->swl();
    }
    // clang-format off
    VS_MAKE_BxDFSet_ASSIGNMENT(DiffuseBxDFSet)
        // clang-format on

        [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                                 MaterialEvalMode mode,
                                                 const Uint &flag) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
};

class BlackBodyBxDFSet : public BxDFSet {
private:
    const SampledWavelengths *swl_{nullptr};

public:
    explicit BlackBodyBxDFSet(const SampledWavelengths &swl) : swl_(&swl) {}
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::Diffuse; }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
    [[nodiscard]] SampledSpectrum principled_albedo(const Float &cos_theta) const noexcept override {
        return {swl_->dimension(), 0.f};
    }
    VS_MAKE_BxDFSet_ASSIGNMENT(BlackBodyBxDFSet)
};

class DielectricBxDFSet : public BxDFSet {
private:
    DCSP<Fresnel> fresnel_;
    MicrofacetReflection refl_;
    MicrofacetTransmission trans_;
    Bool dispersive_{};

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(fresnel_->type_hash(), refl_.type_hash(), trans_.type_hash());
    }

public:
    DielectricBxDFSet(const SP<Fresnel> &fresnel,
                      MicrofacetReflection refl,
                      MicrofacetTransmission trans,
                      Bool dispersive,
                      const Uint &flag)
        : fresnel_(fresnel),
          refl_(ocarina::move(refl)), trans_(ocarina::move(trans)),
          dispersive_(ocarina::move(dispersive)) {}
    VS_MAKE_BxDFSet_ASSIGNMENT(DielectricBxDFSet)
        [[nodiscard]] const SampledWavelengths *swl() const override { return &refl_.swl(); }
    [[nodiscard]] SampledSpectrum principled_albedo(const Float &cos_theta) const noexcept override;
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override { return dispersive_; }
    [[nodiscard]] Bool splittable() const noexcept override { return true; }
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                             const Uint &flag) const noexcept override;
    [[nodiscard]] Uint flag() const noexcept override { return refl_.flags() | trans_.flags(); }
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, const Uint &flag,
                                             TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo, TSampler &sampler) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler) const noexcept override;
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
        [[nodiscard]] SampledSpectrum principled_albedo(const Float &cos_theta) const noexcept override;
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

}// namespace vision
