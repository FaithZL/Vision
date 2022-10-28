//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "sample.h"

namespace vision {

class BxDF {
protected:
    Uchar _flag;

public:
    explicit BxDF(Uchar flag) : _flag(flag) {}
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Float3 eval(Float3 wo, Float3 wi) const noexcept = 0;
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
    explicit LambertReflection(Float3 kr) : BxDF(BxDFFlag::DiffRefl), Kr(kr) {}
    [[nodiscard]] Float3 eval(Float3 wo, Float3 wi) const noexcept override;
};

}// namespace vision