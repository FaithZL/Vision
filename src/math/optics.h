//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "core/vs_header.h"
#include "math/complex.h"

namespace vision {
using namespace ocarina;

inline namespace optics {
template<typename T>
requires ocarina::is_vector3_v<expr_value_t<T>>
[[nodiscard]] condition_t<float3, T> reflect(const T &wo, const T &n) noexcept {
    return -wo + 2 * dot(wo, n) * n;
}

template<EPort p = D>
[[nodiscard]] tuple<oc_bool<p>, oc_float3<p>>
refract(oc_float3<p> wi, oc_float3<p> n, oc_float<p> eta) noexcept {
    oc_float<p> cos_theta_i = dot(n, wi);
//    oc_assert(cos_theta_i > 0, "refract error ! {}", cos_theta_i);
    oc_float<p> sin_theta_i_2 = max(0.f, 1 - sqr(cos_theta_i));
    oc_float<p> sin_theta_t_2 = sin_theta_i_2 / sqr(eta);
    oc_bool<p> valid = select(sin_theta_t_2 >= 1, false, true);

    oc_float<p> cos_theta_t = safe_sqrt(1 - sin_theta_t_2);
    oc_float3<p> wt = -wi / eta + (cos_theta_i / eta - cos_theta_t) * n;
    return {valid, wt};
}

template<typename T>
[[nodiscard]] auto schlick_weight(const T &cos_theta) noexcept {
    condition_t<float, T> m = clamp(1.f - cos_theta, 0.f, 1.f);
    return Pow<5>(m);
}

template<EPort p = D>
[[nodiscard]] oc_float<p> fresnel_schlick(const oc_float<p> &R0,
                                          const oc_float<p> &cos_theta) noexcept {
    return lerp(schlick_weight(cos_theta), R0, 1.f);
}

template<EPort p = D>
[[nodiscard]] oc_float3<p> fresnel_schlick(const oc_float3<p> &R0,
                                           const oc_float<p> &cos_theta) noexcept {
    return lerp(make_float3(schlick_weight(cos_theta)), R0, make_float3(1.f));
}

template<typename T>
[[nodiscard]] scalar_t<T> schlick_R0_from_eta(const T &eta) {
    return sqr(eta - 1) / sqr(eta + 1);
}

template<EPort p = D>
[[nodiscard]] oc_float<p> fresnel_dielectric_impl(oc_float<p> abs_cos_theta_i, oc_float<p> eta) noexcept {
    oc_float<p> sin_theta_i_2 = 1 - sqr(abs_cos_theta_i);
    oc_float<p> sin_theta_t_2 = sin_theta_i_2 / sqr(eta);
    oc_float<p> cos_theta_t = safe_sqrt(1 - sin_theta_t_2);
    oc_float<p> r_parl = (eta * abs_cos_theta_i - cos_theta_t) / (eta * abs_cos_theta_i + cos_theta_t);
    oc_float<p> r_perp = (abs_cos_theta_i - eta * cos_theta_t) / (abs_cos_theta_i + eta * cos_theta_t);
    return select(sin_theta_t_2 >= 1, 1.f, (sqr(r_parl) + sqr(r_perp)) * 0.5f);
}
VS_MAKE_CALLABLE(fresnel_dielectric)

template<EPort p = D>
[[nodiscard]] oc_float<p> fresnel_complex(oc_float<p> cos_theta_i, Complex<p> eta) noexcept {
    oc_float<p> sin_theta_i_2 = 1 - sqr(cos_theta_i);
    Complex<p> sin_theta_t_2 = sin_theta_i_2 / complex_sqr(eta);
    Complex<p> cos_theta_t = complex_sqrt(1.f - sin_theta_t_2);

    Complex<p> r_parl = (eta * cos_theta_i - cos_theta_t) / (eta * cos_theta_i + cos_theta_t);
    Complex<p> r_perp = (cos_theta_i - eta * cos_theta_t) / (cos_theta_i + eta * cos_theta_t);
    return (norm_sqr(r_parl) + norm_sqr(r_perp)) * .5f;
}

template<EPort p = D>
[[nodiscard]] oc_float<p> fresnel_complex(oc_float<p> cos_theta_i, oc_float<p> eta, oc_float<p> k) noexcept {
    return fresnel_complex<p>(cos_theta_i, Complex<p>(eta, k));
}

template<EPort p = D>
[[nodiscard]] oc_float3<p> fresnel_complex(oc_float<p> cos_theta_i, oc_float3<p> eta, oc_float3<p> k) noexcept {
    oc_float3<p> ret;
    for (int i = 0; i < 3; ++i) {
        ret[i] = fresnel_complex<p>(cos_theta_i, eta[i], k[i]);
    }
    return ret;
}

}// namespace optics

}// namespace vision