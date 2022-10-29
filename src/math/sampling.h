//
// Created by Zero on 29/10/2022.
//

#pragma once

#include "core/stl.h"
#include "dsl/common.h"

namespace vision {

template<EPort p = D>
[[nodiscard]] oc_float<p> remapping(const oc_float<p> &a,
                                    const oc_float<p> &low,
                                    const oc_float<p> &high) {
    return (a - low) / (high - low);
}
}// namespace vision