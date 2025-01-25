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
    virtual Fresnel &operator=(const Fresnel &other) noexcept = default;
    [[nodiscard]] virtual SampledSpectrum eta() const noexcept {
        OC_ERROR("ior only dielectric material !");
        return {swl_->dimension(), 1.f};
    }
    virtual void correct_eta(Float cos_theta) noexcept {
        OC_ERROR("correct_eta only dielectric material !");
    }
    [[nodiscard]] virtual SP<Fresnel> clone() const noexcept = 0;
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
    void correct_eta(Float cos_theta) noexcept override {
        eta_ = select(cos_theta > 0, eta_, rcp(eta_));
    }
    [[nodiscard]] Float evaluate(const Float &cos_theta, uint channel) const noexcept override {
        return fresnel_dielectric<D>(cos_theta, eta_[channel]);
    }
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        SampledSpectrum fr = eta_.map([&](const Float &eta) { return fresnel_dielectric<D>(abs_cos_theta, eta); });
        return fr;
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override { return eta_; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelDielectric>(eta_, *swl_);
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelDielectric)
};

class FresnelNoOp : public Fresnel {
public:
    explicit FresnelNoOp(const SampledWavelengths &swl) : Fresnel(swl) {}
    [[nodiscard]] SampledSpectrum evaluate(Float cos_theta) const noexcept override { return {swl_->dimension(), 1.f}; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelNoOp>(*swl_);
    }
    VS_MAKE_Fresnel_ASSIGNMENT(FresnelNoOp)
};

}// namespace vision