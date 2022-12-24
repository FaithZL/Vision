//
// Created by Zero on 21/12/2022.
//

#pragma once

#include "dsl/common.h"

namespace vision {
using namespace ocarina;

static constexpr float3x3 xyz2srgb = make_float3x3(
    +3.240479f, -0.969256f, +0.055648f,
    -1.537150f, +1.875991f, -0.204043f,
    -0.498535f, +0.041556f, +1.057311f);

static constexpr float3x3 srgb2xyz = make_float3x3(
    0.412453f, 0.212671f, 0.019334f,
    0.357580f, 0.715160f, 0.119193f,
    0.180423f, 0.072169f, 0.950227f);

class RGBSigmoidPolynomial {
private:
    array<Float, 3> _c;

private:
    [[nodiscard]] static Float _s(Float x) noexcept {
        return select(isinf(x), cast<float>(x > 0.0f),
                      0.5f * fma(x, rsqrt(fma(x, x, 1.f)), 1.f));
    }

public:
    RGBSigmoidPolynomial() noexcept = default;
    RGBSigmoidPolynomial(Float c0, Float c1, Float c2) noexcept
        : _c{c0, c1, c2} {}
    explicit RGBSigmoidPolynomial(Float3 c) noexcept : _c{c[0], c[1], c[2]} {}
    [[nodiscard]] Float operator()(Float lambda) const noexcept {
        return _s(fma(lambda, fma(lambda, _c[0], _c[1]), _c[2]));// c0 * x * x + c1 * x + c2
    }
};

}// namespace vision