//
// Created by Zero on 21/12/2022.
//

#include "spectrum.h"

namespace vision {

Bool SampledWavelengths::secondary_valid() const noexcept {
    if (dimension() == 1) {
        return false;
    }
    Bool ret = true;
    for (uint i = 1; i < dimension(); ++i) {
        ret = ret & (pdf(i) != 0.f);
    }
    return ret;
}

void SampledWavelengths::invalidation_secondary() noexcept {
    if (dimension() == 1) {
        return;
    }
    $if(secondary_valid()) {
        for (uint i = 1; i < dimension(); ++i) {
            _pdfs[i] = 0.f;
        }
    };
}

Uint SampledWavelengths::valid_dimension() const noexcept {
    Uint ret = 0;
    for (int i = 0; i < dimension(); ++i) {
        ret += ocarina::select(pdf(i) > 0.f, 1, 0);
    }
    return ret;
}

SampledSpectrum select(const SampledSpectrum &p, const SampledSpectrum &t, const SampledSpectrum &f) noexcept {
    uint n = std::max({p.dimension(), t.dimension(), f.dimension()});
    OC_ASSERT((p.dimension() == 1u || p.dimension() == n) &&
              (t.dimension() == 1u || t.dimension() == n) &&
              (f.dimension() == 1u || f.dimension() == n));
    auto r = SampledSpectrum{n};
    for (uint i = 0u; i < n; i++) { r[i] = select(p[i] != 0.f, t[i], f[i]); }
    return r;
}

SampledSpectrum select(const SampledSpectrum &p, const Float &t, const SampledSpectrum &f) noexcept {
    OC_ASSERT(f.dimension() == 1u || p.dimension() == 1u || f.dimension() == p.dimension());
    auto r = SampledSpectrum{std::max(f.dimension(), p.dimension())};
    for (uint i = 0u; i < r.dimension(); i++) { r[i] = select(p[i] != 0.f, t, f[i]); }
    return r;
}

SampledSpectrum select(const SampledSpectrum &p, const SampledSpectrum &t, const Float &f) noexcept {
    OC_ASSERT(t.dimension() == 1u || p.dimension() == 1u || t.dimension() == p.dimension());
    auto r = SampledSpectrum{std::max(t.dimension(), p.dimension())};
    for (uint i = 0u; i < r.dimension(); i++) { r[i] = select(p[i] != 0.f, t[i], f); }
    return r;
}

SampledSpectrum select(const Bool &p, const SampledSpectrum &t, const SampledSpectrum &f) noexcept {
    OC_ASSERT(t.dimension() == 1u || f.dimension() == 1u || t.dimension() == f.dimension());
    auto r = SampledSpectrum{std::max(t.dimension(), f.dimension())};
    for (uint i = 0u; i < r.dimension(); i++) { r[i] = select(p, t[i], f[i]); }
    return r;
}

SampledSpectrum select(const Bool &p, const Float &t, const SampledSpectrum &f) noexcept {
    return f.map([p, t](auto i, auto x) noexcept { return select(p, t, x); });
}

SampledSpectrum select(const Bool &p, const SampledSpectrum &t, const Float &f) noexcept {
    return t.map([p, f](auto i, auto x) noexcept { return select(p, x, f); });
}

SampledSpectrum zero_if_any_nan(const SampledSpectrum &t) noexcept {
    Bool any_nan = t.any([](const Float &value) { return ocarina::isnan(value); });
    return t.map([&any_nan](const Float &x) noexcept { return select(any_nan, 0.f, x); });
}

}// namespace vision