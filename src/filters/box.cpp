//
// Created by Zero on 10/09/2022.
//

#include "description/descriptions.h"
#include "base/filter.h"

namespace vision {

class BoxFilter : public Filter {
public:
    explicit BoxFilter(FilterDesc *desc) : Filter(desc) {}
    [[nodiscard]] FilterSample sample(Float2 u) const noexcept override {
        Float2 p = make_float2(lerp(u[0], -_radius.x, _radius.x),
                               lerp(u[1], -_radius.y, _radius.y));
        return {p, 1.f};
    }
    [[nodiscard]] float evaluate(float2 p) const noexcept override {
        return (std::abs(p.x) <= _radius.x && std::abs(p.y) <= _radius.y) ? 1 : 0;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BoxFilter)