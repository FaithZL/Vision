//
// Created by Zero on 21/12/2022.
//

#pragma once

#include "dsl/common.h"
#include "cie.h"
#include "base/node.h"

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

class SampledSpectrum {
private:
    Array<float> _samples;

public:
    SampledSpectrum(uint n, const Float &value) noexcept
        : _samples(n) {
        for (int i = 0; i < n; ++i) {
            _samples[i] = value;
        }
    }
    explicit SampledSpectrum(uint n) noexcept : SampledSpectrum{n, 0.f} {}
    explicit SampledSpectrum(const Float &value) noexcept : SampledSpectrum{1u, value} {}
    explicit SampledSpectrum(float value) noexcept : SampledSpectrum{1u, value} {}
};

class Spectrum : public Node {
public:
    using Desc = SpectrumDesc;
public:
    explicit Spectrum(const SpectrumDesc &desc) : Node(desc) {}
};

}// namespace vision