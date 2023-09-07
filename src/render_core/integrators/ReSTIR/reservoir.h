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
    oc_float<p> weight_sum;
    oc_float3<p> value;

public:
    void update(oc_float<p> u, oc_float<p> weight, oc_float3<p> v) {
        weight_sum += weight;
        value = select(u < (weight / weight_sum), v, value);
    }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, value) {
    static constexpr EPort p = D;
    void update(oc_float<p> u, oc_float<p> weight, oc_float3<p> v) {
        weight_sum += weight;
        value = select(u < (weight / weight_sum), v, value);
    }
};