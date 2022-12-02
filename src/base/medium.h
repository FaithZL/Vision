//
// Created by Zero on 29/11/2022.
//

#pragma once

#include "dsl/common.h"
#include "math/optics.h"

namespace vision {
using namespace ocarina;
template<EPort p = D>
[[nodiscard]] Float phase_HG(Float cos_theta, Float g) {
    Float denom = 1 + sqr(g) + 2 * g * cos_theta;
    return Inv4Pi * (1 - sqr(g)) / (denom * sqrt(denom));
}

class PhaseFunction {
public:
    [[nodiscard]] virtual Float f(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual pair<Float, Float3> sample_f(Float3 wo, Float2 u) const noexcept = 0;
};

class HenyeyGreenstein : public PhaseFunction {
private:
    Float _g;

public:
    explicit HenyeyGreenstein(Float g) : _g(g) {}
    [[nodiscard]] Float f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] pair<Float, Float3> sample_f(Float3 wo, Float2 u) const noexcept override;
};

}// namespace vision