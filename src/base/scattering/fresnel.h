//
// Created by Zero on 09/11/2022.
//

#pragma once

#include "math/optics.h"
#include "dsl/dsl.h"
#include "core/stl.h"

namespace vision {
class Pipeline;

class Fresnel {
protected:
    const SampledWavelengths &_swl;
    const Pipeline *_rp{};

public:
    explicit Fresnel(const SampledWavelengths &swl, const Pipeline *rp) : _swl(swl), _rp(rp) {}
    [[nodiscard]] virtual SampledSpectrum evaluate(Float cos_theta) const noexcept = 0;
    [[nodiscard]] virtual Float evaluate(Float cos_theta, Uint channel) const noexcept {
        OC_ERROR("Fresnel evaluate by channel invalid !");
        return 0.f;
    }
    [[nodiscard]] virtual SampledSpectrum eta() const noexcept {
        OC_ERROR("ior only dielectric material !");
        return {_swl.dimension(), 1.f};
    }
    virtual void correct_eta(Float cos_theta) noexcept {
        OC_ERROR("correct_eta only dielectric material !");
    }
    [[nodiscard]] virtual SP<Fresnel> clone() const noexcept = 0;
};

class FresnelDielectric : public Fresnel {
private:
    SampledSpectrum _eta;

public:
    explicit FresnelDielectric(const SampledSpectrum &ior, const SampledWavelengths &swl, const Pipeline *rp)
        : Fresnel(swl, rp),
          _eta(ior) {}
    void correct_eta(Float cos_theta) noexcept override {
        _eta = select(cos_theta > 0, _eta, rcp(_eta));
    }
    [[nodiscard]] Float evaluate(ocarina::Float cos_theta, ocarina::Uint channel) const noexcept override {
        return fresnel_dielectric<D>(cos_theta, _eta[channel]);
    }
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        SampledSpectrum fr = _eta.map([&](const Float &eta) { return fresnel_dielectric<D>(abs_cos_theta, eta); });
        return fr;
    }
    [[nodiscard]] SampledSpectrum eta() const noexcept override { return _eta; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelDielectric>(_eta, _swl, _rp);
    }
};

class FresnelNoOp : public Fresnel {
public:
    explicit FresnelNoOp(const SampledWavelengths &swl, const Pipeline *rp) : Fresnel(swl, rp) {}
    [[nodiscard]] SampledSpectrum evaluate(Float cos_theta) const noexcept override { return {_swl.dimension(), 1.f}; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelNoOp>(_swl, _rp);
    }
};

}// namespace vision