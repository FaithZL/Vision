//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/dsl.h"
#include "base/node.h"
#include "descriptions/node_desc.h"

namespace vision {
using namespace ocarina;

struct FilterSample {
    Float2 p;
    Float weight;
};

//"filter": {
//    "type": "box",
//    "name": "boxFilter",
//    "param": {
//        "radius": [
//            0.5,
//            0.5
//        ]
//    }
//}
class Filter {
public:
    class Impl : public Node, public Serializable<float> {
    public:
        using Desc = FilterDesc;
    protected:
        Serial<float2> radius_;

    public:
        explicit Impl(const FilterDesc &desc)
            : Node(desc),
              radius_(desc["radius"].as_float2(make_float2(1.5f))) {}
        OC_SERIALIZABLE_FUNC(Serializable<float>, radius_)
        bool render_UI(ocarina::Widgets *widgets) noexcept override;
        [[nodiscard]] virtual FilterSample sample(Float2 u) const noexcept = 0;
        [[nodiscard]] virtual float evaluate(float2 p) const noexcept = 0;
        [[nodiscard]] vector<float> discretize(uint width) const noexcept;
        template<EPort p = D>
        [[nodiscard]] auto radius() const noexcept {
            if constexpr (p == D) {
                return *radius_;
            } else {
                return radius_.hv();
            }
        }
    };

private:
    SP<Filter::Impl> _impl;

public:
    Filter() = default;
    explicit Filter(const FilterDesc &desc) : _impl(load<Filter>(desc)) {}
    void init(const FilterDesc &desc) noexcept{
        _impl = load<Filter>(desc);
    }
    [[nodiscard]] auto get() const noexcept { return _impl.get(); }
    [[nodiscard]] auto get() noexcept { return _impl.get(); }
    [[nodiscard]] auto operator->() const noexcept { return _impl.get(); }
    [[nodiscard]] auto operator->() noexcept { return _impl.get(); }
};

}// namespace vision