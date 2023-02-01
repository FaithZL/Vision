//
// Created by Zero on 09/11/2022.
//

#pragma once

#include "math/optics.h"
#include "dsl/common.h"
#include "core/stl.h"

namespace vision {
class RenderPipeline;

class Fresnel {
protected:
    const SampledWavelengths &_swl;
    const RenderPipeline *_rp{};

public:
    explicit Fresnel(const SampledWavelengths &swl, const RenderPipeline *rp) : _swl(swl), _rp(rp) {}
    [[nodiscard]] virtual SampledSpectrum evaluate(Float cos_theta) const noexcept = 0;
    [[nodiscard]] virtual Float eta() const noexcept {
        OC_ERROR("ior only dielectric material !");
        return 1;
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
    explicit FresnelDielectric(const SampledSpectrum &ior, const SampledWavelengths &swl, const RenderPipeline *rp)
        : Fresnel(swl, rp), _eta(ior[0]) {}
    void correct_eta(Float cos_theta) noexcept override {
        _eta = select(cos_theta > 0, _eta, rcp(_eta));
    }
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        Float fr = fresnel_dielectric<D>(abs_cos_theta, _eta[0]);
        return {_swl.dimension(), fr};
    }
    [[nodiscard]] Float eta() const noexcept override { return _eta[0]; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelDielectric>(_eta, _swl, _rp);
    }
};

class FresnelNoOp : public Fresnel {
public:
    explicit FresnelNoOp(const SampledWavelengths &swl, const RenderPipeline *rp) : Fresnel(swl, rp) {}
    [[nodiscard]] SampledSpectrum evaluate(Float cos_theta) const noexcept override { return {_swl.dimension(), 1.f}; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelNoOp>(_swl, _rp);
    }
};

}// namespace vision