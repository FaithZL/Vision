//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "dsl/common.h"
#include "base/sample.h"
#include "microfacet.h"
#include "math/optics.h"
#include "fresnel.h"
#include "base/sampler.h"
#include "base/color/spectrum.h"

namespace vision {
using namespace ocarina;

[[nodiscard]] inline SampledSpectrum fresnel_complex(Float cos_theta_i, const SampledSpectrum &eta,
                                                     const SampledSpectrum &k) noexcept {
    SampledSpectrum ret{3u};
    for (uint i = 0; i < ret.dimension(); ++i) {
        ret[i] = fresnel_complex<D>(cos_theta_i, eta[i], k[i]);
    }
    return ret;
}

[[nodiscard]] inline SampledSpectrum fresnel_schlick(const SampledSpectrum &R0,
                                                     const Float &cos_theta) noexcept {
    return lerp(schlick_weight(cos_theta), R0, 1.f);
}

class BxDF {
protected:
    uchar _flag;
    const SampledWavelengths *_swl;

public:
    explicit BxDF(const SampledWavelengths &swl, uchar flag = BxDFFlag::Unset) : _flag(flag), _swl(&swl) {}
    BxDF(const BxDF &other) noexcept : _flag(other._flag), _swl(other._swl) {}
    BxDF &operator=(const BxDF &other) noexcept {
        _flag = other._flag;
        _swl = other._swl;
        return *this;
    }
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept = 0;
    [[nodiscard]] virtual Float3 albedo() const noexcept = 0;
    [[nodiscard]] virtual Bool safe(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual ScatterEval evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual ScatterEval safe_evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] Uchar flag() const noexcept { return _flag; }
    [[nodiscard]] Bool match_flag(Uchar bxdf_flag) const noexcept {
        return ((_flag & bxdf_flag) == _flag);
    }
};

class LambertReflection : public BxDF {
private:
    VSColor Kr;

public:
    explicit LambertReflection(Float3 kr, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          Kr(kr) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return Kr; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
};

class MicrofacetReflection : public BxDF {
private:
    VSColor Kr;
    SP<Microfacet<D>> _microfacet;

public:
    MicrofacetReflection() = default;
    MicrofacetReflection(VSColor color, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : BxDF(swl, BxDFFlag::Reflection), Kr(color),
          _microfacet(m) {}

    [[nodiscard]] VSColor albedo() const noexcept override { return Kr; }
    [[nodiscard]] VSColor f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept override;
};

class MicrofacetTransmission : public BxDF {
private:
    VSColor Kt;
    SP<Microfacet<D>> _microfacet;

public:
    MicrofacetTransmission() = default;
    MicrofacetTransmission(Float3 color, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : BxDF(swl, BxDFFlag::Transmission), Kt(color), _microfacet(m) {}
    [[nodiscard]] Bool safe(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] VSColor albedo() const noexcept override { return Kt; }
    [[nodiscard]] VSColor f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;

    [[nodiscard]] BSDFSample sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept override;
};

}// namespace vision