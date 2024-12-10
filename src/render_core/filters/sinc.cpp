//
// Created by Zero on 2023/7/7.
//

#include "fitted_curve.h"
#include "math/util.h"

namespace vision {
using namespace ocarina;

class LanczosSincFilter : public FittedCurveFilter {
private:
    float tau_{};

public:
    LanczosSincFilter() = default;
    VS_HOTFIX_MAKE_RESTORE(FittedCurveFilter, tau_)
    explicit LanczosSincFilter(const FilterDesc &desc)
        : FittedCurveFilter(desc),
          tau_(desc["tau"].as_float(3.f)) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] float evaluate(ocarina::float2 p) const noexcept override {
        return windowed_sinc<H>(p.x, radius_.hv(), tau_) *
               windowed_sinc<H>(p.y, radius_.hv(), tau_) * 4;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, LanczosSincFilter)
VS_REGISTER_CURRENT_PATH(0, "vision-filter-sinc.dll")