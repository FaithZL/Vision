//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;

template<EPort p = D>
class WeightedReservoir {
private:
    oc_float<p> _weight_sum{};
    oc_float3<p> _value{};
    oc_uint<p> _sample_num{};

public:
    WeightedReservoir() = default;
    void sample(oc_float<p> u, oc_float<p> weight, oc_float3<p> value) {
        _sample_num += 1;
        _weight_sum += weight;
        _value = select(u < (weight / _weight_sum), value, _value);
    }
};

}// namespace vision