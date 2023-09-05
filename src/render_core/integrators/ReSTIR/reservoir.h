//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;

template<typename T, EPort p = D>
class WeightedReservoir {
private:
    oc_float<p> _weight_sum{};
    T _value{};
    oc_uint<p> _sample_num{};

public:
    WeightedReservoir() = default;
    void sample(oc_float<p> u, oc_float<p> weight, T value) {
        _sample_num += 1;
        _weight_sum += weight;
        _value = select(u < (weight / _weight_sum), value, _value);
    }
};

}// namespace vision