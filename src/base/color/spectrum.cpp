//
// Created by Zero on 21/12/2022.
//

#include "spectrum.h"
#include "base/scattering/material.h"

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

void SampledWavelengths::invalidation_secondary() const noexcept {
    if (dimension() == 1) {
        return;
    }
    $if(secondary_valid()) {
        for (uint i = 1; i < dimension(); ++i) {
            pdfs_[i] = 0.f;
        }
    };
}

void SampledWavelengths::check_dispersive(const TSpectrum &spectrum,
                                          const MaterialEvaluator &bsdf) const noexcept {
    if (auto dispersive = spectrum->is_dispersive(&bsdf)) {
        $if(*dispersive) {
            invalidation_secondary();
        };
    }
}

Uint SampledWavelengths::valid_dimension() const noexcept {
    Uint ret = pdfs_.reduce(0u, [&](Uint num, auto f) {
        return num + ocarina::select(f > 0.f, 1, 0);
    });
    return ret;
}

bool Spectrum::render_UI(ocarina::Widgets *widgets) noexcept {
    string label = ocarina::format("{} spectrum", impl_type().data());
    return widgets->use_folding_header(label, [&] {
        render_sub_UI(widgets);
    });
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

SampledSpectrum zero_if_any_nan_inf(const SampledSpectrum &t) noexcept {
    Bool any_nan_inf = t.any([](const Float &value) { return ocarina::isnan(value) || ocarina::isinf(value); });
    return t.map([&any_nan_inf](const Float &x) noexcept { return select(any_nan_inf, 0.f, x); });
}

SampledSpectrum safe_div(const SampledSpectrum &lhs, const SampledSpectrum &rhs) noexcept {
    return lhs.map([&](uint i, const Float &a) {
        return safe_div(a, rhs[i]);
    });
}

}// namespace vision