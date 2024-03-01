//
// Created by Zero on 24/02/2024.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"

namespace vision {
using namespace ocarina;
struct SVGFData {
    float4 illumi_v{};
    float history{};
    float2 moments{};
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::SVGFData, illumi_v, history, moments) {
    [[nodiscard]] Float variance() const noexcept {
       return illumi_v.w;
    }
    [[nodiscard]] Float3 illumination() const noexcept {
       return illumi_v.xyz();
    }
};
// clang-format on