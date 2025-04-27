//
// Created by Zero on 09/11/2022.
//

#pragma once

#include "math/optics.h"
#include "dsl/dsl.h"
#include "core/stl.h"

namespace vision {
class Pipeline;

class Fresnel : public ocarina::Hashable {
protected:
    const SampledWavelengths *swl_{};

public:
    explicit Fresnel(const SampledWavelengths &swl) : swl_(&swl) {}
    [[nodiscard]] virtual SampledSpectrum evaluate(Float cos_theta) const noexcept = 0;
    [[nodiscard]] virtual Float evaluate(const Float &cos_theta, uint channel) const noexcept {
        OC_ERROR("Fresnel evaluate by channel invalid !");
        return 0.f;
    }
    OC_MAKE_MEMBER_GETTER(swl, )
    virtual Fresnel &operator=(const Fresnel &other) noexcept = default;
    [[nodiscard]] virtual SampledSpectrum eta() const noexcept {
        OC_ERROR("ior only dielectric material !");
        return {swl_->dimension(), 1.f};
    }
    virtual void set_eta(const SampledSpectrum &eta) noexcept {
        OC_NOT_IMPLEMENT_ERROR(Fresnel::set_eta);
    }
};

#define VS_MAKE_FRESNEL_ASSIGNMENT(ClassName)                              \
    ClassName &operator=(const Fresnel &other) noexcept override {         \
        *this = *dynamic_cast<ClassName *>(const_cast<Fresnel *>(&other)); \
        return *this;                                                      \
    }

class FresnelSchlick : public Fresnel {
private:
    SampledSpectrum F0_;
    SampledSpectrum eta_;

public:
    FresnelSchlick(SampledSpectrum F0, SampledSpectrum eta,
                   const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)),
          eta_(std::move(eta)) {}
    FresnelSchlick(SampledSpectrum F0, const Float &eta,
                   const SampledWavelengths &swl)
        : Fresnel(swl), F0_(std::move(F0)),
          eta_(SampledSpectrum{1, eta}) {}
    OC_MAKE_MEMBER_GETTER(F0, )
    void set_eta(const vision::SampledSpectrum &eta) noexcept override {
        eta_ = eta;
    }
    [[nodiscard]] SampledSpectrum evaluate(ocarina::Float cos_theta) const noexcept override {
        Float F_real = fresnel_dielectric(cos_theta, eta_[0]);
        Float F0_real = schlick_F0_from_ior(eta_[0]);
        Float t = inverse_lerp(F_real, F0_real, 1.f);
        t = ocarina::clamp(t, 0.f, 1.f);
        SampledSpectrum ret = lerp(t, F0_, 1.f);
        return ret;
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override {
        return eta_;
    }
    VS_MAKE_FRESNEL_ASSIGNMENT(FresnelSchlick)
};

class FresnelDielectric : public Fresnel {
private:
    SampledSpectrum eta_;

public:
    explicit FresnelDielectric(const SampledSpectrum &ior, const SampledWavelengths &swl)
        : Fresnel(swl),
          eta_(ior) {}
    [[nodiscard]] Float evaluate(const Float &cos_theta, uint channel) const noexcept override {
        return fresnel_dielectric<D>(cos_theta, eta_[channel]);
    }
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        SampledSpectrum fr = eta_.map([&](const Float &eta) {
            return fresnel_dielectric<D>(abs_cos_theta, eta);
        });
        return fr;
    }
    void set_eta(const vision::SampledSpectrum &eta) noexcept override {
        eta_ = eta;
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override { return eta_; }
    VS_MAKE_FRESNEL_ASSIGNMENT(FresnelDielectric)
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

    void init_from_F82(const SampledSpectrum &F82) {
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
    VS_MAKE_FRESNEL_ASSIGNMENT(FresnelF82Tint)
};

class FresnelConstant : public Fresnel {
public:
    explicit FresnelConstant(const SampledWavelengths &swl) : Fresnel(swl) {}
    [[nodiscard]] SampledSpectrum evaluate(Float cos_theta) const noexcept override { return {swl_->dimension(), 1.f}; }
    VS_MAKE_FRESNEL_ASSIGNMENT(FresnelConstant)
};

}// namespace vision