//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "dsl/common.h"
#include "sample.h"
#include "microfacet.h"
#include "math/optics.h"

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
    Microfacet<D> _microfacet;
    FresnelType _fresnel_type;

public:
    explicit MicrofacetReflection(Float3 color, Float ax, Float ay,
                                  FresnelType fresnel_type,
                                  MicrofacetType microfacet_type = GGX)
        : BxDF(BxDFFlag::Reflection), Kr(color),
          _microfacet(ax, ay, microfacet_type),
          _fresnel_type(fresnel_type) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return Kr; }
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] Float PDF(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] BSDFSample sample(Float3 wo, Float2 u) const noexcept override;
};

}// namespace vision