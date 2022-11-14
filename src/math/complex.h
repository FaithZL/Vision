//
// Created by Zero on 14/11/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"

namespace vision {
template<EPort p = D>
struct Complex {
public:
    oc_float<p> re{};
    oc_float<p> im{};

public:
    explicit Complex(oc_float<p> re, oc_float<p> im = 0.f) : re(re), im(im) {}
    [[nodiscard]] Complex<p> operator-() const { return {-re, -im}; }
    [[nodiscard]] Complex<p> operator+(Complex<p> z) const { return Complex<p>{re + z.re, im + z.im}; }
    [[nodiscard]] Complex<p> operator-(Complex<p> z) const { return Complex<p>{re - z.re, im - z.im}; }
    [[nodiscard]] Complex<p> operator*(Complex<p> z) const {
        return Complex<p>(re * z.re - im * z.im, re * z.im + im * z.re);
    }
    [[nodiscard]] Complex<p> operator/(Complex<p> z) const {
        oc_float<p> scale = 1 / (z.re * z.re + z.im * z.im);
        return Complex<p>(scale * (re * z.re + im * z.im), scale * (im * z.re - re * z.im));
    }
};

template<EPort p = D>
[[nodiscard]] oc_float<p> norm_sqr(const Complex<p> &z) {
    return z.re * z.re + z.im * z.im;
}

template<EPort p = D>
[[nodiscard]] Complex<p> complex_sqr(const Complex<p> &z) {
    return z * z;
}

template<EPort p = D>
[[nodiscard]] oc_float<p> norm(const Complex<p> &z) {
    return sqrt(norm_sqr(z));
}

template<EPort p = D>
[[nodiscard]] oc_float<p> abs(const Complex<p> &z) {
    return norm(z);
}

template<EPort p = D>
[[nodiscard]] Complex<p> complex_sqrt(const Complex<p> &z) {
    oc_float<p> n = abs(z);
    oc_float<p> t1 = sqrt(0.5f * (n + abs(z.re)));
    oc_float<p> t2 = 0.5f * z.im / t1;
    Complex<p> ret{0};
    ret.re = select(n == 0, 0.f, select(z.re >= 0, t1, abs(t2)));
    ret.im = select(n == 0, 0.f, select(z.re >= 0, t2, copysign(t1, z.im)));
    return ret;
}

}// namespace vision

template<ocarina::EPort p = ocarina::EPort::D>
[[nodiscard]] vision::Complex<p> operator+(ocarina::oc_float<p> value, vision::Complex<p> z) {
    return vision::Complex<p>(value) + z;
}

template<ocarina::EPort p = ocarina::EPort::D>
[[nodiscard]] vision::Complex<p> operator-(ocarina::oc_float<p> value, vision::Complex<p> z) {
    return vision::Complex<p>(value) - z;
}

template<ocarina::EPort p = ocarina::EPort::D>
[[nodiscard]] vision::Complex<p> operator*(ocarina::oc_float<p> value, vision::Complex<p> z) {
    return vision::Complex<p>(value) * z;
}

template<ocarina::EPort p = ocarina::EPort::D>
[[nodiscard]] vision::Complex<p> operator*(vision::Complex<p> z, ocarina::oc_float<p> value) {
    return vision::Complex<p>(value) * z;
}

template<ocarina::EPort p = ocarina::EPort::D>
[[nodiscard]] vision::Complex<p> operator/(ocarina::oc_float<p> value, vision::Complex<p> z) {
    return vision::Complex<p>(value) / z;
}