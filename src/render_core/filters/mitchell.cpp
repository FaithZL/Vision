//
// Created by Zero on 2023/7/7.
//

#include "fitted_curve.h"
#include "math/util.h"

namespace vision {

class MitchellFilter : public FittedCurveFilter {
private:
    float _b;
    float _c;

public:
    explicit MitchellFilter(const FilterDesc &desc)
        : FittedCurveFilter(desc),
          _b(desc["b"].as_float(1.f / 3.f)),
          _c(desc["c"].as_float(1.f / 3.f)) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] float mitchell_1d(float x) const {
        x = ocarina::abs(x);
        if (x <= 1)
            return ((12 - 9 * _b - 6 * _c) * Pow<3>(x) + (-18 + 12 * _b + 6 * _c) * ocarina::sqr(x) +
                    (6 - 2 * _b)) *
                   (1.f / 6.f);
        else if (x <= 2)
            return ((-_b - 6 * _c) * Pow<3>(x) + (6 * _b + 30 * _c) * ocarina::sqr(x) +
                    (-12 * _b - 48 * _c) * x + (8 * _b + 24 * _c)) *
                   (1.f / 6.f);
        else
            return 0;
    }

    [[nodiscard]] float evaluate(ocarina::float2 p) const noexcept override {
        return mitchell_1d(2 * p.x / _radius.hv().x) *
               mitchell_1d(2 * p.y / _radius.hv().y);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MitchellFilter)