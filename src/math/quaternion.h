//
// Created by Zero on 2024/3/30.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "geometry.h"

namespace vision {

template<EPort p>
class oc_quaternion {
private:
    oc_float4<p> _vw{make_float4(0, 0, 0, 1)};

public:
    oc_quaternion() = default;
    explicit oc_quaternion(oc_float4<p> val) : _vw(val) {}
    explicit oc_quaternion(oc_float3<p> v, oc_float<p> w) : _vw(make_float4(v, w)) {}

    static oc_quaternion<p> &_from_float3x3_trace_gt0(oc_float3x3<p> m, oc_float<p> trace) {
        oc_float<p> s = sqrt(trace + 1.0f);
        oc_float<p> w = s * 0.5f;
        s = 0.5f / s;
        oc_float3<p> v;
        v.x = (m[2][1] - m[1][2]) * s;
        v.y = (m[0][2] - m[2][0]) * s;
        v.z = (m[1][0] - m[0][1]) * s;
        return oc_quaternion<p>(v, w);
    }

    static oc_quaternion<p> from_float3x3(oc_float3x3<p> m) {
        oc_quaternion<p> ret;
        oc_float<p> t = trace(m);
        oc_float<p> s = sqrt(1.f + t);

        return ret;
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
