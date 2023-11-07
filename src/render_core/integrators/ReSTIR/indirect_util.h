//
// Created by Zero on 2023/11/7.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"
#include "base/sampler.h"

namespace vision::ReSTIRIndirect {

struct RSVSample {
    array<float, 3> pos;
    array<float, 3> ng;
};

}// namespace vision::ReSTIRIndirect