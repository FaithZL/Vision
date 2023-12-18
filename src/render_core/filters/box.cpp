//
// Created by Zero on 10/09/2022.
//

#include "descriptions/node_desc.h"
#include "base/sensor/filter.h"

namespace vision {

class BoxFilter : public Filter {
public:
    explicit BoxFilter(const FilterDesc &desc) : Filter(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] FilterSample sample(Float2 u) const noexcept override {
        Float2 p = make_float2(lerp(u[0], -radius().x, radius().x),
                               lerp(u[1], -radius().y, radius().y));
        return {p, 1.f};
    }
    [[nodiscard]] float evaluate(float2 p) const noexcept override {
        return (std::abs(p.x) <= _radius.hv().x && std::abs(p.y) <= _radius.hv().y) ? 1 : 0;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BoxFilter)