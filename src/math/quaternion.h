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
    static oc_quaternion<p> from_float3x3(oc_float3x3<p> m) {
        oc_quaternion<p> ret;
        oc_float<p> t = trace(m);
        if (t > 0.f) {
            oc_float<p> s = sqrt(t + 1.0f);
            oc_float<p> w = s * 0.5f;
            s = 0.5f / s;
            oc_float3<p> v;
            v.x = (m[1][2] - m[2][1]) * s;
            v.y = (m[2][0] - m[0][2]) * s;
            v.z = (m[0][1] - m[1][0]) * s;
            return oc_quaternion<p>(v, w);
        } else {
            const int nxt[3] = {1, 2, 0};
            float q[3];
            int i = 0;
            if (m[1][1] > m[0][0]) {
                i = 1;
            }
            if (m[2][2] > m[i][i]) {
                i = 2;
            }
            int j = nxt[i];
            int k = nxt[j];
            float s = std::sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);
            q[i] = s * 0.5f;
            if (s != 0.f) {
                s = 0.5f / s;
            }
            float w;
            float3 v;
            w = (m[j][k] - m[k][j]) * s;
            q[j] = (m[i][j] + m[j][i]) * s;
            q[k] = (m[i][k] + m[k][i]) * s;
            v.x = q[0];
            v.y = q[1];
            v.z = q[2];
            return oc_quaternion<p>(v, w);
        }
    }
    oc_float3x3<p> to_float3x3() const noexcept {
        oc_float<p> xx = v().x * v().x, yy = v().y * v().y, zz = v().z * v().z;
        oc_float<p> xy = v().x * v().y, xz = v().x * v().z, yz = v().y * v().z;
        oc_float<p> wx = v().x * w(), wy = v().y * w(), wz = v().z * w();
        oc_float3x3<p> m = make_float3x3(1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy),
                                       2 * (xy + wz), 1 - 2 * (xx + zz), 2 * (yz - wx),
                                       2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy));
        return m;
    }
    oc_quaternion &operator+=(const oc_quaternion &q) {
        _vw += q._vw;
        return *this;
    }
    oc_float3<p> v() const noexcept {
        return _vw.xyz();
    }
    oc_float<p> w() const noexcept {
        return _vw.w;
    }
    oc_float<p> theta() const noexcept {
        oc_float<p> half_theta = safe_acos(w());
        return half_theta * 2.f;
    }
    oc_float3<p> axis() const noexcept {
        oc_float<p> half_theta = safe_acos(w());
        oc_float<p> s = sin(half_theta);
        return v() / s;
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
