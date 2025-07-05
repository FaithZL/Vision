//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/dsl.h"
#include "base/node.h"
#include "core/node_desc.h"

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
class Filter : public Node, public Encodable {
public:
    using Desc = FilterDesc;

protected:
    EncodedData<float2> radius_;

public:
    Filter() = default;
    explicit Filter(const FilterDesc &desc)
        : Node(desc),
          radius_(make_float2(desc["radius"].as_float(0.5f))) {}
    OC_ENCODABLE_FUNC(Encodable, radius_)
    VS_HOTFIX_MAKE_RESTORE(Node, radius_)

    //todo rebuild function for FittedCurveFilter
    virtual void rebuild() noexcept {}
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

using TFilter = TObjectUI<Filter>;

}// namespace vision