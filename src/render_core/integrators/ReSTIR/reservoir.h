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
    oc_float<p> _value{};

public:
    
};

}