//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "dsl/common.h"

namespace vision {

using namespace ocarina;

struct SensorSample {
    Float2 p_film;
    Float2 p_lens;
    Float time;
    Float filter_weight{1.f};
};

struct RaySample {
    OCRay ray;
    Float weight;
};

}// namespace vision