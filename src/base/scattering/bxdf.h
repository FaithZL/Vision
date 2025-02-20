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
    [[nodiscard]] virtual SampledSpectrum f(const Float3 &wo, const Float3 &wi, SP<Fresnel> fresnel,
                                            TransportMode tm = TransportMode::Radiance) const noexcept = 0;
    [[nodiscard]] virtual SampledSpectrum albedo(const Float &cos_theta) const noexcept = 0;
    [[nodiscard]] virtual Bool valid(const Float3 &wo, const Float3 &wi, const Float3 &wh) const noexcept;
    [[nodiscard]] virtual ScatterEval evaluate(const Float3 &wo, const Float3 &wi,
                                               SP<Fresnel> fresnel, MaterialEvalMode mode,
                                               TransportMode tm = TransportMode::Radiance) const noexcept;
    [[nodiscard]] virtual ScatterEval safe_evaluate(const Float3 &wo, const Float3 &wi,
                                                    SP<Fresnel> fresnel,MaterialEvalMode mode,
                                                    TransportMode tm = TransportMode::Radiance) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(const Float3 &wo, TSampler &sampler,SP<Fresnel> fresnel,
                                            TransportMode tm = TransportMode::Radiance) const noexcept;
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
        [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override { return Kr; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi,
                                    SP<Fresnel> fresnel,TransportMode tm) const noexcept override {
        return Kr * InvPi;
    }
};

class MicrofacetBxDF : public BxDF {
protected:
    DCSP<Microfacet<D>> microfacet_;

public:
    MicrofacetBxDF() = default;
    MicrofacetBxDF(const SP<Microfacet<D>> &microfacet, uint flag,
                   const SampledWavelengths &swl)
        : BxDF(swl, flag),
          microfacet_(microfacet) {}
    [[nodiscard]] Float alpha_x() const noexcept { return microfacet_->alpha_x(); }
    [[nodiscard]] Float alpha_y() const noexcept { return microfacet_->alpha_y(); }
    [[nodiscard]] Microfacet<D> *microfacet() noexcept { return microfacet_.get(); }
    [[nodiscard]] const Microfacet<D> *microfacet() const noexcept { return microfacet_.get(); }
    [[nodiscard]] Float alpha_average() const noexcept {
        return sqrt(alpha_x() * alpha_y());
    }
    void set_alpha(const Float2 &alpha) noexcept {
        microfacet_->set_alpha_x(alpha.x);
        microfacet_->set_alpha_y(alpha.y);
    }
    void set_alpha(const Float &alpha) noexcept {
        microfacet_->set_alpha_x(alpha);
        microfacet_->set_alpha_y(alpha);
    }
};

class MicrofacetReflection : public MicrofacetBxDF {
private:
    SampledSpectrum kr_;

public:
    VS_MAKE_BxDF_ASSIGNMENT(MicrofacetReflection)
        MicrofacetReflection() = default;
    MicrofacetReflection(SampledSpectrum color, const SampledWavelengths &swl, const SP<Microfacet<D>> &m)
        : MicrofacetBxDF(m, BxDFFlag::GlossyRefl, swl), kr_(std::move(color)) {}
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override { return kr_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi,
                                    SP<Fresnel> fresnel,TransportMode tm) const noexcept override;
    [[nodiscard]] Float PDF(const Float3 &wo, const Float3 &wi,
                            SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(const Float3 &wo, Float2 u,
                                             SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] BSDFSample sample(const Float3 &wo, TSampler &sampler,
                                    SP<Fresnel> fresnel,TransportMode tm) const noexcept override;
};

class OrenNayar : public BxDF {
private:
    SampledSpectrum R_;
    Float A_, B_;

public:
    OrenNayar(SampledSpectrum R, Float sigma, const SampledWavelengths &swl);
    VS_MAKE_BxDF_ASSIGNMENT(OrenNayar)
        [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override { return R_; }
    [[nodiscard]] SampledSpectrum f(const Float3 &wo, const Float3 &wi,SP<Fresnel> fresnel,
                                    TransportMode tm) const noexcept override;
};

}// namespace vision