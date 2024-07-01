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
class FilterImpl : public Node, public Encodable<float> {
public:
    using Desc = FilterDesc;

protected:
    EncodedData<float2> radius_;

public:
    explicit FilterImpl(const FilterDesc &desc)
        : Node(desc),
          radius_(desc["radius"].as_float2(make_float2(1.5f))) {}
    OC_SERIALIZABLE_FUNC(Encodable<float>, radius_)
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

using Filter = TObjectUI<FilterImpl>;

}// namespace vision