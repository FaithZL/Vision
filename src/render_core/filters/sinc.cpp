//
// Created by Zero on 2023/7/7.
//

#include "fitted_curve.h"
#include "math/util.h"

namespace vision {
using namespace ocarina;

class LanczosSincFilter : public FittedCurveFilter {
private:
    float _tau{};

public:
    explicit LanczosSincFilter(const FilterDesc &desc)
        : FittedCurveFilter(desc),
          _tau(desc["tau"].as_float(3.f)) {}

    [[nodiscard]] float evaluate(ocarina::float2 p) const noexcept override {
        return windowed_sinc<H>(p.x, _radius.hv().x, _tau) *
               windowed_sinc<H>(p.y, _radius.hv().y, _tau) * 4;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::LanczosSincFilter)