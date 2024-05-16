//
// Created by Zero on 24/02/2024.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"

namespace vision::svgf {
template<EPort p = D>
[[nodiscard]] oc_float3<p> demodulate_impl(const oc_float3<p> &c,
                                           const oc_float3<p> &albedo) {
    return c / ocarina::max(albedo, make_float3(0.001f, 0.001f, 0.001f));
}
VS_MAKE_CALLABLE(demodulate)

using namespace ocarina;
struct SVGFData {
    float4 illumi_v{};
    float history{};
    float2 moments{};
};
}// namespace vision::svgf

// clang-format off
OC_STRUCT(vision::svgf, SVGFData, illumi_v, history, moments) {
    [[nodiscard]] Float variance() const noexcept {
       return illumi_v.w;
    }
    [[nodiscard]] Float3 illumination() const noexcept {
       return illumi_v.xyz();
    }
};
// clang-format on

namespace vision::svgf {
using namespace ocarina;
using SVGFDataVar = Var<SVGFData>;

}// namespace vision::svgf