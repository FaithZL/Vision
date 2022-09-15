//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "ocarina/src/core/basic_types.h"
#include "ocarina/src/dsl/common.h"

namespace vision {
using namespace ocarina;

inline namespace optics {
template<typename T>
requires ocarina::is_vector3_v<expr_value_t<T>>
[[nodiscard]] T reflect(const T &wo, const T &n) {
    return -wo + 2 * dot(wo, n) * n;
}

template<typename T>
[[nodiscard]] T schlick_weight(const T &cos_theta) {
    T m = clamp(1.f - cos_theta, 0.f, 1.f);
    return Pow<5>(m);
}

template<typename T, typename U>
[[nodiscard]] auto fresnel_schlick(const T &R0, const U &cos_theta) {
    return lerp(schlick_weight(cos_theta), R0, T{1.f});
}

}// namespace optics

}// namespace vision