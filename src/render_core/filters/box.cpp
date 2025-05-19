//
// Created by Zero on 10/09/2022.
//

#include "core/node_desc.h"
#include "base/sensor/filter.h"
#include "hotfix/hotfix.h"

namespace vision {

class BoxFilter : public Filter {
public:
    BoxFilter() = default;
    explicit BoxFilter(const FilterDesc &desc) : Filter(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] FilterSample sample(Float2 u) const noexcept override {
        Float2 p = make_float2(lerp(u[0], -radius().x, radius().x),
                               lerp(u[1], -radius().y, radius().y));
        return {p, 1.f};
    }
    [[nodiscard]] float evaluate(float2 p) const noexcept override {
        return (std::abs(p.x) <= radius_.hv().x && std::abs(p.y) <= radius_.hv().y) ? 1 : 0;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, BoxFilter)
//VS_REGISTER_CURRENT_PATH(0, "vision-filter-box.dll")