//
// Created by Zero on 20/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "core/constants.h"
#include "core/vs_header.h"

namespace vision {

using namespace ocarina;

template<EPort p = D>
[[nodiscard]] oc_float<p> remapping(const oc_float<p> &a,
                                    const oc_float<p> &low,
                                    const oc_float<p> &high) {
    return (a - low) / (high - low);
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float2<p> square_to_disk_impl(const oc_float2<p> &u) {
    oc_float<p> r = sqrt(u.x);
    oc_float<p> theta = ocarina::_2Pi * u.y;
    return make_float2(r * cos(theta), r * sin(theta));
}
VS_MAKE_CALLABLE(square_to_disk)

[[nodiscard]] inline float uniform_disk_PDF() {
    return ocarina::InvPi;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> square_to_cosine_hemisphere_impl(const oc_float2<p> &u) {
    oc_float2<p> d = square_to_disk<p>(u);
    oc_float<p> z = sqrt(max(0.0f, 1.0f - d.x * d.x - d.y * d.y));
    return make_float3(d.x, d.y, z);
}
VS_MAKE_CALLABLE(square_to_cosine_hemisphere)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> cosine_hemisphere_PDF_impl(const oc_float<p> &cos_theta) {
    return cos_theta * ocarina::InvPi;
}
VS_MAKE_CALLABLE(cosine_hemisphere_PDF)

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> square_to_cone_impl(const oc_float2<p> &u, oc_float<p> &cos_theta_max) {
    oc_float<p> cos_theta = (1 - u.x) + u.x * cos_theta_max;
    oc_float<p> sin_theta = sqrt(1 - cos_theta * cos_theta);
    oc_float<p> phi = ocarina::_2Pi * u.y;
    return make_float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}
VS_MAKE_CALLABLE(square_to_cone)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> uniform_cone_PDF_impl(const oc_float<p> &cos_theta_max) {
    return 1 / (ocarina::_2Pi * (1 - cos_theta_max));
}
VS_MAKE_CALLABLE(uniform_cone_PDF)

template<EPort p = EPort::D>
[[nodiscard]] oc_float2<p> square_to_triangle_impl(const oc_float2<p> &u) {
    oc_float<p> su0 = sqrt(u.x);
    return make_float2(1 - su0, u.y * su0);
}
VS_MAKE_CALLABLE(square_to_triangle)

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> square_to_sphere_impl(const oc_float2<p> &u) {
    oc_float<p> z = 1 - 2 * u[0];
    oc_float<p> r = sqrt(max(0.f, 1 - z * z));
    oc_float<p> phi = 2 * Pi * u[1];
    return make_float3(r * cos(phi), r * sin(phi), z);
}
VS_MAKE_CALLABLE(square_to_sphere)

[[nodiscard]] inline float uniform_sphere_PDF() {
    return ocarina::Inv4Pi;
}

/**
 * p(dir) = p(pos) * r^2 / cos(theta)
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_dir_impl(const oc_float<p> &PDF_pos, const oc_float3<p> &normal,
                                       const oc_float3<p> &wo_un) {
    oc_float<p> cos_theta = abs(dot(normal, normalize(wo_un)));
    return PDF_pos * length_squared(wo_un) / cos_theta;
}
VS_MAKE_CALLABLE(PDF_dir)

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> square_to_hemisphere_impl(const oc_float2<p> &sample) {
    oc_float<p> z = sample.x;
    oc_float<p> tmp = safe_sqrt(1.0f - z * z);
    oc_float<p> phi = 2.0f * ocarina::Pi * sample.y;
    oc_float<p> sin_phi = sin(phi);
    oc_float<p> cos_phi = cos(phi);
    return make_float3(cos_phi * tmp, sin_phi * tmp, z);
}
VS_MAKE_CALLABLE(square_to_hemisphere)

[[nodiscard]] inline float uniform_hemisphere_PDF() {
    return ocarina::Inv2Pi;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> linear_PDF_impl(oc_float<p> x, oc_float<p> a, oc_float<p> b) {
    //    assert(a > 0 && b > 0);
    oc_float<p> ret = 2 * lerp(x, a, b) / (a + b);
    return select(x < 0.f || x > 1.f, 0.f, ret);
}
VS_MAKE_CALLABLE(linear_PDF)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> sample_linear_impl(oc_float<p> u, oc_float<p> a, oc_float<p> b) {
    //    assert(a >= 0 && b >= 0);
    oc_float<p> x = u * (a + b) / (a + sqrt(lerp(u, sqr(a), sqr(b))));
    oc_float<p> ret = min(x, OneMinusEpsilon);
    return select(u == 0 && a == 0, 0.f, ret);
}
VS_MAKE_CALLABLE(sample_linear)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> sample_tent_impl(oc_float<p> u, oc_float<p> r) {
    return select(u < 0.5f,
                  -r * sample_linear<p>((0.5f - u) * 2, 1, 0),
                  r * sample_linear<p>((u - 0.5f) * 2, 1, 0));
}
VS_MAKE_CALLABLE(sample_tent)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> balance_heuristic_impl(const oc_int<p> &nf,
                                                 const oc_float<p> &f_PDF,
                                                 const oc_int<p> &ng,
                                                 const oc_float<p> &g_PDF) {
    return (nf * f_PDF) / (nf * f_PDF + ng * g_PDF);
}
VS_MAKE_CALLABLE(balance_heuristic)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> power_heuristic_impl(const oc_int<p> &nf,
                                               const oc_float<p> &f_PDF,
                                               const oc_int<p> &ng,
                                               const oc_float<p> &g_PDF) {
    oc_float<p> f = nf * f_PDF, g = ng * g_PDF;
    return (f * f) / (f * f + g * g);
}
VS_MAKE_CALLABLE(power_heuristic)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> mis_weight_impl(const oc_float<p> &f_PDF,
                                          const oc_float<p> &g_PDF) {
    return balance_heuristic<p>(1, f_PDF, 1, g_PDF);
}
VS_MAKE_CALLABLE(mis_weight)

[[nodiscard]] Array<float> mis_weight(const Array<float> &f_PDF, const Array<float> &g_PDF) {
    return f_PDF / (f_PDF + g_PDF);
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> robust_mis_weight_impl(const oc_float<p> &f_PDF,
                                                 const oc_float<p> &g_PDF) {
    return select(f_PDF > g_PDF,
                  1.f / (1.f + sqr(g_PDF / f_PDF)),
                  1.f - 1.f / (1.f + sqr(f_PDF / g_PDF)));
}
VS_MAKE_CALLABLE(robust_mis_weight)

}// namespace vision