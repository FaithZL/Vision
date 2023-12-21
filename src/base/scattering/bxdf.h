//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "dsl/dsl.h"
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
    SampledSpectrum ret{eta.dimension()};
    for (uint i = 0; i < ret.dimension(); ++i) {
        ret[i] = fresnel_complex<D>(cos_theta_i, eta[i], k[i]);
    }
    return ret;
}

[[nodiscard]] inline SampledSpectrum fresnel_schlick(const SampledSpectrum &R0,
                                                     const Float &cos_theta) noexcept {
    return lerp(schlick_weight(cos_theta), R0, 1.f);
}

class BxDF : public ocarina::Hashable {
protected:
    uint _flags{};
    const SampledWavelengths *_swl{};

public:
    BxDF() = default;
    explicit BxDF(const SampledWavelengths &swl, uint flag) : _flags(flag), _swl(&swl) {}
    BxDF(const BxDF &other) = default;
    virtual BxDF &operator=(const BxDF &other) noexcept = default;
    virtual void regularize() noexcept {}
    [[nodiscard]] const SampledWavelengths &swl() const noexcept { return *_swl; }
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept = 0;
    [[nodiscard]] virtual SampledSpectrum albedo() const noexcept = 0;
    [[nodiscard]] virtual Bool safe(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual ScatterEval evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual ScatterEval safe_evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] Uint flags() const noexcept { return _flags; }
    [[nodiscard]] Bool match_flag(const Uint &bxdf_flag) const noexcept {
        return ((_flags & bxdf_flag) == _flags);
    }
    virtual ~BxDF() = default;
};

class LambertReflection : public BxDF {
private:
    SampledSpectrum Kr;

public:
    explicit LambertReflection(SampledSpectrum kr, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          Kr(kr) {}
    LambertReflection &operator=(const BxDF &other) noexcept override {
        *this = dynamic_cast<decltype(*this) &>(const_cast<BxDF &>(other));
        return *this;
    }
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return Kr; }
    [[nodiscard]] SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        return Kr * InvPi;
    }
};

class MicrofacetReflection : public BxDF {
private:
    SampledSpectrum Kr;
    SP<Microfacet<D>> _microfacet;

public:
    MicrofacetReflection() = default;
    MicrofacetReflection(SampledSpectrum color, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : BxDF(swl, BxDFFlag::GlossyRefl), Kr(color),
          _microfacet(m) {}

    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return Kr; }
    [[nodiscard]] SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept override;
};

class MicrofacetTransmission : public BxDF {
private:
    SampledSpectrum Kt;
    SP<Microfacet<D>> _microfacet;

public:
    MicrofacetTransmission() = default;
    MicrofacetTransmission(SampledSpectrum color, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : BxDF(swl, BxDFFlag::GlossyTrans), Kt(color), _microfacet(m) {}
    [[nodiscard]] Bool safe(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return Kt; }
    [[nodiscard]] SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept override;
};

}// namespace vision