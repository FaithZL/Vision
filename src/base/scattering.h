//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "dsl/common.h"
#include "sample.h"
#include "microfacet.h"
#include "math/optics.h"
#include "fresnel.h"

namespace vision {
using namespace ocarina;

class BxDF {
protected:
    uchar _flag;

public:
    explicit BxDF(uchar flag = BxDFFlag::Unset) : _flag(flag) {}
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept = 0;
    [[nodiscard]] virtual Float3 albedo() const noexcept = 0;
    [[nodiscard]] virtual Bool safe(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual BSDFEval evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual BSDFEval safe_evaluate(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual ScatterSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] virtual SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept;
    [[nodiscard]] Uchar flag() const noexcept { return _flag; }
    [[nodiscard]] Bool match_flag(Uchar bxdf_flag) const noexcept {
        return ((_flag & bxdf_flag) == _flag);
    }
};

class LambertReflection : public BxDF {
private:
    Float3 Kr;

public:
    explicit LambertReflection(Float3 kr)
        : BxDF(BxDFFlag::DiffRefl),
          Kr(kr) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return Kr; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
};

class MicrofacetReflection : public BxDF {
private:
    Float3 Kr;
    SP<Microfacet<D>> _microfacet;

public:
    MicrofacetReflection() = default;
    MicrofacetReflection(Float3 color, const SP<Microfacet<D>> &m)
        : BxDF(BxDFFlag::Reflection), Kr(color),
          _microfacet(m) {}

    [[nodiscard]] Float3 albedo() const noexcept override { return Kr; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] ScatterSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
};

class MicrofacetTransmission : public BxDF {
private:
    Float3 Kt;
    SP<Microfacet<D>> _microfacet;

public:
    MicrofacetTransmission() = default;
    MicrofacetTransmission(Float3 color, const SP<Microfacet<D>> &m)
        : BxDF(BxDFFlag::Transmission), Kt(color), _microfacet(m) {}
    [[nodiscard]] Bool safe(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] Float3 albedo() const noexcept override { return Kt; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
    [[nodiscard]] ScatterSample sample(Float3 wo, Float2 u, SP<Fresnel> fresnel) const noexcept override;
};

}// namespace vision