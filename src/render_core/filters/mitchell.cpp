//
// Created by Zero on 2023/7/7.
//

#include "fitted_curve.h"
#include "math/util.h"

namespace vision {

class MitchellFilter : public FittedCurveFilter {
private:
    float b_{};
    float c_{};

public:
    MitchellFilter() = default;
    VS_HOTFIX_MAKE_RESTORE(FittedCurveFilter, b_, c_)
    explicit MitchellFilter(const FilterDesc &desc)
        : FittedCurveFilter(desc),
          b_(desc["b"].as_float(1.f / 3.f)),
          c_(desc["c"].as_float(1.f / 3.f)) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        changed_ |= widgets->drag_float("b", &b_, 0.01, 0.01, 5);
        changed_ |= widgets->drag_float("c", &c_, 0.01, 0.01, 5);
        check_rebuild(changed_);
    }
    [[nodiscard]] float mitchell_1d(float x) const {
        x = ocarina::abs(x);
        if (x <= 1)
            return ((12 - 9 * b_ - 6 * c_) * Pow<3>(x) + (-18 + 12 * b_ + 6 * c_) * ocarina::sqr(x) +
                    (6 - 2 * b_)) *
                   (1.f / 6.f);
        else if (x <= 2)
            return ((-b_ - 6 * c_) * Pow<3>(x) + (6 * b_ + 30 * c_) * ocarina::sqr(x) +
                    (-12 * b_ - 48 * c_) * x + (8 * b_ + 24 * c_)) *
                   (1.f / 6.f);
        else
            return 0;
    }

    [[nodiscard]] float evaluate(ocarina::float2 p) const noexcept override {
        return mitchell_1d(2 * p.x / radius_.hv().x) *
               mitchell_1d(2 * p.y / radius_.hv().y);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, MitchellFilter)
//VS_REGISTER_CURRENT_PATH(0, "vision-filter-mitchell.dll")