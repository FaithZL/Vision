//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "dsl/common.h"
#include "sample.h"

namespace vision {
using namespace ocarina;


class BxDF {
protected:
    //todo
    Uchar _flag;

public:
    explicit BxDF(Uchar flag) : _flag(flag) {}
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Float3 f(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual Bool safe(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Evaluation evaluate(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Evaluation safe_evaluate(Float3 wo, Float3 wi) const noexcept;
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
        : BxDF(select(is_zero(kr), BxDFFlag::Unset, BxDFFlag::DiffRefl)),
          Kr(kr) {}
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi) const noexcept override;
};

}// namespace vision