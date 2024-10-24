//
// Created by Zero on 28/10/2022.
//

#pragma once

#include <utility>

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

OC_MAKE_ENUM_BIT_OPS(vision::MaterialEvalMode, |, &, <<, >>)

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
    uint flags_{};
    const SampledWavelengths *swl_{};

public:
    BxDF() = default;
    explicit BxDF(const SampledWavelengths &swl, uint flag) : flags_(flag), swl_(&swl) {}
    BxDF(const BxDF &other) = default;
    virtual BxDF &operator=(const BxDF &other) noexcept = default;
    virtual void regularize() noexcept {}
    [[nodiscard]] const SampledWavelengths &swl() const noexcept { return *swl_; }
    [[nodiscard]] virtual Float PDF(const Float3 &wo, const Float3 &wi,
                                    SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual SampledSpectrum f(const Float3 &wo, const Float3 &wi,
                                            SP<Fresnel> fresnel) const noexcept = 0;
    [[nodiscard]] virtual SampledSpectrum albedo(const Float3 &wo) const noexcept = 0;
    [[nodiscard]] virtual Bool safe(const Float3 &wo, const Float3 &wi) const noexcept;
    [[nodiscard]] virtual ScatterEval evaluate(const Float3 &wo, const Float3 &wi,
                                               SP<Fresnel> fresnel,
                                               MaterialEvalMode mode) const noexcept;
    [[nodiscard]] virtual ScatterEval safe_evaluate(const Float3 &wo, const Float3 &wi,
                                                    SP<Fresnel> fresnel,
                                                    MaterialEvalMode mode) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(const Float3 &wo, TSampler &sampler,
                                            SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual SampledDirection sample_wi(const Float3 &wo, Float2 u,
                                                     SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] Uint flags() const noexcept { return flags_; }
    [[nodiscard]] static bool match_F(MaterialEvalMode mode) noexcept {
        return static_cast<bool>(mode & MaterialEvalMode::F);
    }
    [[nodiscard]] static bool match_PDF(MaterialEvalMode mode) noexcept {
        return static_cast<bool>(mode & MaterialEvalMode::PDF);
    }
    [[nodiscard]] Bool match_flag(const Uint &bxdf_flag) const noexcept {
        return ((flags_ & bxdf_flag) == flags_);
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
          Kr(std::move(kr)) {}
    VS_MAKE_BxDF_ASSIGNMENT(LambertReflection)
        [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return Kr; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi,
                                    SP<Fresnel> fresnel) const noexcept override {
        return Kr * InvPi;
    }
};

class MicrofacetReflection : public BxDF {
private:
    SampledSpectrum kr_;
    DCSP<Microfacet<D>> microfacet_;

public:
    VS_MAKE_BxDF_ASSIGNMENT(MicrofacetReflection)
        MicrofacetReflection() = default;
    MicrofacetReflection(SampledSpectrum color, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : BxDF(swl, BxDFFlag::GlossyRefl), kr_(std::move(color)),
          microfacet_(m) {}
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return kr_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi,
                                    SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(const Float3 &wo, const Float3 &wi,
                            SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, Float2 u,
                                             SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(const Float3 &wo, TSampler &sampler,
                                    SP<Fresnel> fresnel) const noexcept override;
};

class MicrofacetTransmission : public BxDF {
private:
    SampledSpectrum kt_;
    DCSP<Microfacet<D>> microfacet_;

public:
    VS_MAKE_BxDF_ASSIGNMENT(MicrofacetTransmission)
        MicrofacetTransmission() = default;
    MicrofacetTransmission(SampledSpectrum color, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : BxDF(swl, BxDFFlag::GlossyTrans), kt_(std::move(color)), microfacet_(m) {}
    [[nodiscard]] Bool safe(const Float3 &wo, const Float3 &wi) const noexcept override;
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return kt_; }
    [[nodiscard]] Float BTDF(const Float3 &wo, const Float3 &wi,
                             SP<Fresnel> fresnel, uint channel) const noexcept;
    [[nodiscard]] SampledSpectrum BTDF(const Float3 &wo, const Float3 &wi,
                                       SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi,
                                    SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledSpectrum f_array(const Float3 &wo, const Float3 &wi,
                                          SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] Float PDF(const Float3 &wo, const Float3 &wi,
                            SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel,
                            uint channel) const noexcept;
    [[nodiscard]] float_array PDF_array(const Float3 &wo, const Float3 &wi,
                                        SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, Float2 u,
                                             SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(const Float3 &wo, TSampler &sampler,
                                    SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] ScatterEval safe_evaluate(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel,
                                            MaterialEvalMode mode) const noexcept override;
};

}// namespace vision