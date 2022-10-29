//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "dsl/common.h"

namespace vision {
using namespace ocarina;
struct BSDFSample {
    Float3 val;
    Float pdf;
    Float3 wi;
    Uchar flags;
};

class BxDF {
public:
    struct Flag {
        static constexpr uchar Unset = 1;
        static constexpr uchar Reflection = 1 << 1;
        static constexpr uchar Transmission = 1 << 2;
        static constexpr uchar Diffuse = 1 << 3;
        static constexpr uchar Glossy = 1 << 4;
        static constexpr uchar Specular = 1 << 5;
        static constexpr uchar NearSpec = 1 << 6;
        // Composite _BxDFFlags_ definitions
        static constexpr uchar DiffRefl = Diffuse | Reflection;
        static constexpr uchar DiffTrans = Diffuse | Transmission;
        static constexpr uchar GlossyRefl = Glossy | Reflection;
        static constexpr uchar GlossyTrans = Glossy | Transmission;
        static constexpr uchar SpecRefl = Specular | Reflection;
        static constexpr uchar SpecTrans = Specular | Transmission;
        static constexpr uchar All = Diffuse | Glossy | Specular | Reflection | Transmission | NearSpec;
    };

protected:
    Uchar _flag;

public:
    explicit BxDF(Uchar flag) : _flag(flag) {}
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Float safe_PDF(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Float3 eval(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual Float3 safe_eval(Float3 wo, Float3 wi) const noexcept;
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
        : BxDF(select(is_zero(kr), Flag::Unset, Flag::DiffRefl)),
          Kr(kr) {}
    [[nodiscard]] Float3 eval(Float3 wo, Float3 wi) const noexcept override;
};

}// namespace vision