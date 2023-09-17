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
    float pq;
    float PMF;
    array<float, 3> pos;
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
};
}// namespace vision
// clang-format off
OC_STRUCT(vision::RSVSample, light_index, prim_id, u, pq, PMF, pos) {
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
};
// clang-format on

namespace vision {
using OCRSVSample = Var<RSVSample>;
}

namespace vision {
using namespace ocarina;

struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    RSVSample sample{};
    oc_uint<p> M{};

public:
    void update(oc_float<p> u, oc_float<p> weight, RSVSample v) {
        weight_sum += weight;
        M += 1;
        sample = ocarina::select(u < (weight / M), v, sample);
    }
    [[nodiscard]] auto W() const noexcept {
        return weight_sum / (M * sample.pq);
    }
    void invalidate() noexcept { weight_sum = 0.f; }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, sample, M) {
    static constexpr EPort p = D;
    void update(oc_float<p> u, oc_float<p> weight, vision::OCRSVSample v) {
        weight_sum += weight;
        M += 1;
        sample = select(u < (weight / weight_sum), v, sample);
    }
    [[nodiscard]] auto W() const noexcept {
        return weight_sum / (M * sample.pq);
    }
    void invalidate() noexcept { weight_sum = 0.f; }
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
        ret->update(u, rsv.weight_sum, rsv.sample);
        ret.M += rsv.M;
    }
    return ret;
}

}// namespace vision