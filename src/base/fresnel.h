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
    [[nodiscard]] virtual Float ior() const noexcept {
        OC_ERROR("ior only dielectric material !");
        return 1;
    }
};

class FresnelDielectric : public Fresnel {
private:
    Float _ior;

public:
    explicit FresnelDielectric(Float ior) : _ior(ior) {}

    [[nodiscard]] Float3 evaluate(Float cos_theta) const noexcept override {
        Float fr = fresnel_dielectric<D>(cos_theta, select(cos_theta > 0, _ior, rcp(_ior)));
        return make_float3(fr);
    }
    [[nodiscard]] Float ior() const noexcept override{
        return _ior;
    }
};

class FresnelNoOp : public Fresnel {
public:
    FresnelNoOp() = default;
    [[nodiscard]] Float3 evaluate(Float cos_theta) const noexcept override { return make_float3(1.f); }
};

}// namespace vision