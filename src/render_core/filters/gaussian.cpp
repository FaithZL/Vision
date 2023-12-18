//
// Created by Zero on 2023/7/7.
//

#include "fitted_curve.h"
#include "math/util.h"

namespace vision {

class GaussianFilter : public FittedCurveFilter {
private:
    float _sigma{};
    float _exp_x{};
    float _exp_y{};

public:
    explicit GaussianFilter(const FilterDesc &desc)
        : FittedCurveFilter(desc),
          _sigma(desc["sigma"].as_float(1.f)),
          _exp_x(gaussian<H>(_radius.hv().x, 0, _sigma)),
          _exp_y(gaussian<H>(_radius.hv().y, 0, _sigma)) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] float evaluate(ocarina::float2 p) const noexcept override {
        float vx = gaussian<H>(p.x, 0, _sigma) - _exp_x;
        float vy = gaussian<H>(p.y, 0, _sigma) - _exp_y;
        return ocarina::max(0.f, vx) * ocarina::max(0.f, vy);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GaussianFilter)