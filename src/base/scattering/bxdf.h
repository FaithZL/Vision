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
enum MaterialEvalMode {
    F = 1 << 0,
    PDF = 1 << 1,
    All = F | PDF
};
}

VS_MAKE_ENUM_BIT_OP(vision::MaterialEvalMode, |)
VS_MAKE_ENUM_BIT_OP(vision::MaterialEvalMode, &)
VS_MAKE_ENUM_BIT_OP(vision::MaterialEvalMode, <<)
VS_MAKE_ENUM_BIT_OP(vision::MaterialEvalMode, >>)

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
    [[nodiscard]] ScatterEval evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel,
                                       MaterialEvalMode mode) const noexcept;
    [[nodiscard]] ScatterEval safe_evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel,
                                            MaterialEvalMode mode) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(Float3 wo, Sampler *sampler, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] Uint flags() const noexcept { return _flags; }
    [[nodiscard]] static bool match_f(MaterialEvalMode mode) noexcept {
        return static_cast<bool>(mode & MaterialEvalMode::F);
    }
    [[nodiscard]] static bool match_pdf(MaterialEvalMode mode) noexcept {
        return static_cast<bool>(mode & MaterialEvalMode::PDF);
    }
    [[nodiscard]] Bool match_flag(const Uint &bxdf_flag) const noexcept {
        return ((_flags & bxdf_flag) == _flags);
    }
    virtual ~BxDF() = default;
};

#define VS_MAKE_BxDF_ASSIGNMENT(ClassName)                            \
    ClassName &operator=(const BxDF &other) noexcept override {       \
        *this = dynamic_cast<ClassName &>(const_cast<BxDF &>(other)); \
        return *this;                                                 \
    }

class LambertReflection : public BxDF {
private:
    SampledSpectrum Kr;

public:
    explicit LambertReflection(SampledSpectrum kr, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          Kr(kr) {}
    VS_MAKE_BxDF_ASSIGNMENT(LambertReflection)
        [[nodiscard]] SampledSpectrum albedo() const noexcept override { return Kr; }
    [[nodiscard]] SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        return Kr * InvPi;
    }
};

class MicrofacetReflection : public BxDF {
private:
    SampledSpectrum Kr;
    DCSP<Microfacet<D>> _microfacet;

public:
    VS_MAKE_BxDF_ASSIGNMENT(MicrofacetReflection)
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
    DCSP<Microfacet<D>> _microfacet;

public:
    VS_MAKE_BxDF_ASSIGNMENT(MicrofacetTransmission)
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