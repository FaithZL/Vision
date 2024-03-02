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

namespace vision {
using namespace ocarina;
using SVGFDataVar = Var<SVGFData>;

[[nodiscard]] inline Float cal_weight(const Float &cur_depth, const Float &neighbor_depth, const Float &sigma_depth,
                                      const Float3 &cur_normal, const Float3 &neighbor_normal, const Float &sigma_normal,
                                      const Float &cur_illumi, const Float &neighbor_illumi, const Float &sigma_illumi) noexcept {
    Float ret = 0;
    Float weight_normal = pow(saturate(dot(cur_normal, neighbor_normal)), sigma_normal);
    Float weight_depth = ocarina::select(sigma_depth == 0, 0.f, abs(cur_depth - neighbor_depth) / sigma_depth);
    Float weight_illumi = abs(cur_illumi - neighbor_illumi) / sigma_illumi;
    ret = exp(-(max(weight_illumi, 0.f) + max(weight_depth, 0.f))) * weight_normal;
    return ret;
}

}// namespace vision