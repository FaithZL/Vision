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

enum FresnelType : uint8_t {
    NoOp = 0,
    Dielectric,
    DisneyFr,
    Conductor
};

inline Float3 eval_fresnel(Float cos_theta, FresnelType type) noexcept {
    switch (type) {
        case NoOp:
            return make_float3(1.f);
    }
    return make_float3(1.f);
}

class BxDF {
protected:
    uchar _flag;

public:
    explicit BxDF(uchar flag) : _flag(flag) {}
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Float3 f(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual Float3 albedo() const noexcept = 0;
    [[nodiscard]] virtual Bool safe(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual BSDFEval evaluate(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual BSDFEval safe_evaluate(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual BSDFSample sample(Float3 wo, Float2 u) const noexcept;
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
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi) const noexcept override;
};

class MicrofacetReflection : public BxDF {
private:
    Float3 Kr;
    shared_ptr<Fresnel> _fresnel;
    shared_ptr<Microfacet<D>> _microfacet;

public:
    MicrofacetReflection(Float3 color, const shared_ptr<Microfacet<D>> &m,
                         const shared_ptr<Fresnel> &f)
        : BxDF(BxDFFlag::Reflection), Kr(color),
          _microfacet(m), _fresnel(f) {}

    [[nodiscard]] Float3 albedo() const noexcept override { return Kr; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u) const noexcept override;
};

class MicrofacetTransmission : public BxDF {
private:
    Float3 Kt;
    shared_ptr<Fresnel> _fresnel;
    shared_ptr<Microfacet<D>> _microfacet;

public:
    MicrofacetTransmission(Float3 color, const shared_ptr<Microfacet<D>> &m,
                           const shared_ptr<Fresnel> &f)
        : BxDF(BxDFFlag::Transmission), Kt(color),
          _microfacet(m), _fresnel(f) {}

    [[nodiscard]] Bool safe(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] Float3 albedo() const noexcept override { return Kt; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u) const noexcept override;
};

}// namespace vision