//
// Created by Zero on 09/11/2022.
//

#pragma once

#include "math/optics.h"
#include "dsl/common.h"
#include "core/stl.h"

namespace vision {

class Fresnel {
public:
    [[nodiscard]] virtual Float3 evaluate(Float cos_theta) const noexcept = 0;
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
    Float _eta;

public:
    explicit FresnelDielectric(Float ior) : _eta(ior) {}
    void correct_eta(Float cos_theta) noexcept override {
        _eta = select(cos_theta > 0, _eta, rcp(_eta));
    }
    [[nodiscard]] Float3 evaluate(Float abs_cos_theta) const noexcept override {
        Float fr = fresnel_dielectric<D>(abs_cos_theta, _eta);
        return make_float3(fr);
    }
    [[nodiscard]] Float eta() const noexcept override { return _eta; }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelDielectric>(_eta);
    }
};

class FresnelNoOp : public Fresnel {
public:
    FresnelNoOp() = default;
    [[nodiscard]] Float3 evaluate(Float cos_theta) const noexcept override { return make_float3(1.f); }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelNoOp>();
    }
};

}// namespace vision