//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;

struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_uint<p> value{};
    oc_uint<p> sample_num{};
    oc_float3<p> W{};

public:
    void update(oc_float<p> u, oc_float<p> weight, oc_uint<p> v) {
        weight_sum += weight;
        sample_num += 1;
        value = select(u < (weight / weight_sum), v, value);
    }

    template<typename Func>
    void update_W(Func &&func) noexcept {
        W = weight_sum / cast<float>(sample_num) / func(value);
    }

    void reset_W() noexcept {
        W = make_float3(0.f);
    }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, value, sample_num, W) {
    static constexpr EPort p = D;
    void update(oc_float<p> u, oc_float<p> weight, oc_uint<p> v) {
        weight_sum += weight;
        sample_num += 1;
        value = select(u < (weight / weight_sum), v, value);
    }
    template<typename Func>
    void update_W(Func && func) noexcept {
        W = weight_sum / cast<float>(sample_num) / func(value);
    }
    void reset_W() noexcept {
        W = make_float3(0.f);
    }
};

namespace vision {
using namespace ocarina;

using OCReservoir = Var<Reservoir>;

template<typename Func>
[[nodiscard]] OCReservoir combine_reservoirs(const vector<OCReservoir> &reservoirs,
                                             const vector<Float> &rands, Func &&func) {
    OCReservoir ret;
    for (int i = 0; i < reservoirs.size(); ++i) {
        const OCReservoir &rsv = reservoirs[i];
        Float u = rands[i];
        Float factor = cast<float>(all(rsv.W == 0.f));
        ret->update(u, rsv.weight_sum * factor, rsv.value);
        ret.sample_num += rsv.sample_num;
    }
    ret->update_W(OC_FORWARD(func));
    return ret;
}



}// namespace vision