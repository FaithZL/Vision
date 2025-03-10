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
    OC_MAKE_MEMBER_GETTER(swl,)
    virtual Fresnel &operator=(const Fresnel &other) noexcept = default;
    [[nodiscard]] virtual SampledSpectrum eta() const noexcept {
        OC_ERROR("ior only dielectric material !");
        return {swl_->dimension(), 1.f};
    }
    virtual void set_eta(const SampledSpectrum &eta) noexcept {
        OC_NOT_IMPLEMENT_ERROR(Fresnel::set_eta);
    }
};

#define VS_MAKE_Fresnel_ASSIGNMENT(ClassName)                              \
    ClassName &operator=(const Fresnel &other) noexcept override {         \
        *this = *dynamic_cast<ClassName *>(const_cast<Fresnel *>(&other)); \
        return *this;                                                      \
    }
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
        SampledSpectrum fr = eta_.map([&](const Float &eta) { return fresnel_dielectric<D>(abs_cos_theta, eta); });
        return fr;
    }
    void set_eta(const vision::SampledSpectrum &eta) noexcept override {
        eta_ = eta;
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override { return eta_; }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelDielectric)
};

class FresnelConstant : public Fresnel {
public:
    explicit FresnelConstant(const SampledWavelengths &swl) : Fresnel(swl) {}
    [[nodiscard]] SampledSpectrum evaluate(Float cos_theta) const noexcept override { return {swl_->dimension(), 1.f}; }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelConstant)
};

}// namespace vision