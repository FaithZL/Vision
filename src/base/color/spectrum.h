//
// Created by Zero on 21/12/2022.
//

#pragma once

#include "dsl/common.h"
#include "cie.h"

namespace vision {
using namespace ocarina;

class SampledWavelengths {
private:
    Array<float> _lambdas;
    Array<float> _pdfs;

public:
    explicit SampledWavelengths(uint dim) noexcept : _lambdas{dim}, _pdfs{dim} {}
    [[nodiscard]] auto lambda(const Uint &i) const noexcept { return _lambdas[i]; }
    [[nodiscard]] auto pdf(const Uint &i) const noexcept { return _pdfs[i]; }
    void set_lambda(const Uint &i, const Float &lambda) noexcept { _lambdas[i] = lambda; }
    void set_pdf(const Uint &i, const Float &pdf) noexcept { _pdfs[i] = pdf; }
    [[nodiscard]] uint dimension() const noexcept { return static_cast<uint>(_lambdas.size()); }
};

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