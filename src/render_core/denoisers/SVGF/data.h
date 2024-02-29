//
// Created by Zero on 24/02/2024.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"

namespace vision {
using namespace ocarina;
struct SVGFData {
    float4 illumination{};
    float variance{};
    float history{};
    float2 moments{};
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::SVGFData, illumination, variance, history, moments) {

};
// clang-format on