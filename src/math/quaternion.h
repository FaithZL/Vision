//
// Created by Zero on 2024/3/30.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"

namespace vision {

template<EPort p>
class oc_quaternion {
private:
    oc_float4<p> _vw{make_float4(0, 0, 0, 1)};

public:
    oc_quaternion() = default;
    explicit oc_quaternion(oc_float4x4<p> mat) {
    }
    oc_quaternion &operator+=(const oc_quaternion &q) {
        _vw += q._vw;
        return *this;
    }
    oc_float3<p> v() const noexcept {
        return _vw.xyz();
    }
    oc_float3<p> w() const noexcept {
        return _vw.w;
    }
    friend oc_quaternion<p> operator+(const oc_quaternion<p> &q1, const oc_quaternion<p> &q2) {
        oc_quaternion<p> ret = q1;
        return ret += q2;
    }
    oc_quaternion &operator-=(const oc_quaternion &q) {
        _vw -= q._vw;
        return *this;
    }
    oc_quaternion operator-() const {
        oc_quaternion ret;
        ret._vw = -_vw;
        return ret;
    }
    friend oc_quaternion<p> operator-(const oc_quaternion<p> &q1, const oc_quaternion<p> &q2) {
        oc_quaternion<p> ret = q1;
        return ret -= q2;
    }
    oc_quaternion &operator*=(oc_float<p> f) {
        _vw *= f;
        return *this;
    }
    oc_quaternion operator*(oc_float<p> f) const {
        oc_quaternion ret = *this;
        ret.v *= f;
        return ret;
    }
    oc_quaternion &operator/=(oc_float<p> f) {
        _vw /= f;
        return *this;
    }
    oc_quaternion operator/(oc_float<p> f) const {
        oc_quaternion ret = *this;
        ret.v /= f;
        return ret;
    }
};

using quaternion = oc_quaternion<H>;
using Quaternion = oc_quaternion<D>;

}// namespace vision
