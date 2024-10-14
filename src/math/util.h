//
// Created by Zero on 27/10/2022.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "core/vs_header.h"

namespace vision {

template<EPort p = D, uint N = 4>
[[nodiscard]] oc_uint<p> tea_impl(oc_uint<p> val0, oc_uint<p> val1) {
    oc_uint<p> v0 = val0;
    oc_uint<p> v1 = val1;
    oc_uint<p> s0 = 0;
    for (uint n = 0; n < N; n++) {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}
VS_MAKE_CALLABLE(tea)

template<EPort p = D>
[[nodiscard]] oc_float<p> lcg_impl(oc_uint<p> &state) noexcept {
    constexpr auto lcg_a = 1664525u;
    constexpr auto lcg_c = 1013904223u;
    state = lcg_a * state + lcg_c;
    return ((state & 0x00ffffffu) * 1.f) * (1.f / static_cast<float>(0x01000000u));
}
VS_MAKE_CALLABLE(lcg)

template<EPort p = D>
[[nodiscard]] inline oc_float<p> gaussian_impl(oc_float<p> x,
                                               oc_float<p> mu,
                                               oc_float<p> sigma) {
    return 1.f / ocarina::sqrt(2 * Pi * ocarina::sqr(sigma)) *
           ocarina::exp(-ocarina::sqr(x - mu) / (2 * ocarina::sqr(sigma)));
}
VS_MAKE_CALLABLE(gaussian)

[[nodiscard]] inline float gaussian_integral(float x0, float x1, float mu = 0,
                                             float sigma = 1) {
    float sigmaRoot2 = sigma * float(1.414213562373095);
    return 0.5f * (std::erf((mu - x0) / sigmaRoot2) - std::erf((mu - x1) / sigmaRoot2));
}

template<EPort p = D>
[[nodiscard]] oc_float<p> sin_x_over_x_impl(oc_float<p> x) noexcept {
    return ocarina::select(1 + x * x == 1, 1.f, ocarina::sin(x) / x);
}
VS_MAKE_CALLABLE(sin_x_over_x)

template<EPort p = D>
[[nodiscard]] oc_float<p> sinc(oc_float<p> x) noexcept {
    return sin_x_over_x<p>(Pi * x);
}

template<EPort p = D>
[[nodiscard]] oc_float<p> windowed_sinc_impl(oc_float<p> x, oc_float<p> radius,
                                             oc_float<p> tau) noexcept {
    return ocarina::select(ocarina::abs(x) > radius, 0.f, sinc<p>(x) * sinc<p>(x / tau));
}
VS_MAKE_CALLABLE(windowed_sinc)

inline void line_bresenham(float2 p1, float2 p2,
                           const std::function<void(int, int)> &write) noexcept {
    int px1 = p1.x;
    int py1 = p1.y;

    int px2 = p2.x;
    int py2 = p2.y;

    if (px1 == px2 && py1 == py2) {
        write(px1, py1);
    }

    int dx = ocarina::abs(px2 - px1);
    int dy = ocarina::abs(py2 - py1);

    if (dx >= dy) {
        if (px1 > px2) {
            std::swap(p1, p2);
        }
        px1 = p1.x;
        py1 = p1.y;

        px2 = p2.x;
        py2 = p2.y;

        int sign = py2 >= py1 ? 1 : -1;
        int k = sign * dy * 2;
        int e = -dx * sign;

        for (int x = px1, y = py1; x <= px2; ++x) {
            write(x, y);
            e += k;
            if (sign * e > 0) {
                y += sign;
                e -= 2 * dx * sign;
            }
        }
    } else {
        if (py1 > py2) {
            std::swap(p1, p2);
        }
        px1 = p1.x;
        py1 = p1.y;

        px2 = p2.x;
        py2 = p2.y;

        int sign = px2 > px1 ? 1 : -1;
        int k = sign * dx * 2;
        int e = -dy * sign;

        for (int x = px1, y = py1; y <= py2; ++y) {
            write(x, y);
            e += k;
            if (sign * e > 0) {
                x += sign;
                e -= 2 * dy * sign;
            }
        }
    }
}

inline void safe_line_bresenham(float2 p1, float2 p2,
                                const std::function<void(int, int)> &write) noexcept {
    if (has_nan(p1) || has_nan(p2) || has_inf(p1) || has_inf(p2)) {
        return;
    }
    line_bresenham(p1, p2, write);
}

}// namespace vision