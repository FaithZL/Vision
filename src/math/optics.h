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
[[nodiscard]] T reflect(const T &wo, const T &n) noexcept {
    return -wo + 2 * dot(wo, n) * n;
}

template<EPort p = D>
[[nodiscard]] pair<oc_bool<p>, oc_float3<p>>
refract(oc_float3<p> wi, oc_float3<p> n, oc_float<p> eta) noexcept {
    oc_float<p> cos_theta_i = dot(n, wi);
    oc_assert(cos_theta_i > 0, "refract error ! {}", cos_theta_i);
    oc_float<p> sin_theta_i_2 = max(0, 1 - sqr(cos_theta_i));
    oc_float<p> sin_theta_t_2 = sin_theta_i_2 / sqr(eta);
    oc_bool<p> valid = select(sin_theta_t_2 >= 1, false, true);

    oc_float<p> cos_theta_t = safe_sqrt(1 - sin_theta_t_2);
    oc_float3<p> wt = -wi / eta + (cos_theta_i / eta - cos_theta_t) * n;
    return make_pair(valid, wt);
}

template<typename T>
[[nodiscard]] T schlick_weight(const T &cos_theta) noexcept {
    T m = clamp(1.f - cos_theta, 0.f, 1.f);
    return Pow<5>(m);
}

template<typename T, typename U>
[[nodiscard]] auto fresnel_schlick(const T &R0, const U &cos_theta) noexcept {
    return lerp(schlick_weight(cos_theta), R0, T{1.f});
}

template<EPort p = D>
[[nodiscard]] oc_float<p> fresnel_dielectric(oc_float<p> abs_cos_theta_i, oc_float<p> eta) noexcept {
    oc_float<p> sin_theta_i_2 = 1 - sqr(abs_cos_theta_i);
    oc_float<p> sin_theta_t_2 = sin_theta_i_2 / sqr(eta);
    oc_float<p> cos_theta_t = safe_sqrt(1 - sin_theta_t_2);
    oc_float<p> r_parl = (eta * abs_cos_theta_i - cos_theta_t) / (eta * abs_cos_theta_i + cos_theta_t);
    oc_float<p> r_perp = (abs_cos_theta_i - eta * cos_theta_t) / (abs_cos_theta_i + eta * cos_theta_t);
    return select(sin_theta_t_2 >= 1, 1.f, (sqr(r_parl) + sqr(r_perp)) / 2);
}

}// namespace optics

}// namespace vision