//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;
struct RSVSample {
    uint light_index{};
    uint prim_id{};
    float2 u;
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::RSVSample, light_index, prim_id, u) {};
// clang-format on

namespace vision {
using namespace ocarina;

struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    RSVSample sample{};
    oc_uint<p> M{};
    oc_float<p> W{};

public:
    void update(oc_float<p> u, oc_float<p> weight, RSVSample v) {
        weight_sum += weight;
        M += 1;
        sample = ocarina::select(u < (weight / M), v, sample);
    }

    template<typename Func>
    void update_W(Func &&func) noexcept {
        W = weight_sum / cast<float>(M) / func(sample);
    }

    void reset_W() noexcept {
        W = 0.f;
    }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, sample, M, W) {
    static constexpr EPort p = D;
    void update(oc_float<p> u, oc_float<p> weight, Var<vision::RSVSample> v) {
        weight_sum += weight;
        M += 1;
        sample = select(u < (weight / weight_sum), v, sample);
    }
    template<typename Func>
    void update_W(Func && func) noexcept {
        W = weight_sum / cast<float>(M) / func(sample);
    }
    void reset_W() noexcept {
        W = 0.f;
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
        Float factor = cast<float>(rsv.W == 0.f);
        ret->update(u, rsv.weight_sum * factor, rsv.sample);
        ret.M += rsv.M;
    }
    ret->update_W(OC_FORWARD(func));
    return ret;
}

}// namespace vision