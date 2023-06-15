//
// Created by Zero on 19/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;
inline namespace geometry {

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] auto cos_theta_2(const T &v) noexcept { return sqr(v.z); }

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] auto cos_theta(const T &v) noexcept { return v.z; }

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> abs_cos_theta(const T &v) noexcept { return ocarina::abs(v.z); }

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> sin_theta_2(const T &v) noexcept { return 1.0f - cos_theta_2(v); }

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> sin_theta(const T &v) noexcept {
    scalar_t<T> temp = sin_theta_2(v);
    return select(temp <= 0.f, 0.f, sqrt(temp));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> tan_theta(const T &v) noexcept {
    scalar_t<T> sin_theta2 = 1 - cos_theta_2(v);
    return select(sin_theta2 <= 0.f, 0.f, sqrt(sin_theta2) / cos_theta(v));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> tan_theta_2(const T &v) noexcept {
    scalar_t<T> cos_theta2 = cos_theta_2(v);
    scalar_t<T> sin_theta2 = 1.f - cos_theta2;
    return select(sin_theta2 <= 0.f, 0.f, sin_theta2 / cos_theta2);
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> sin_phi(const T &v) noexcept {
    scalar_t<T> sinTheta = sin_theta(v);
    return select(sinTheta == 0, 1.f, clamp(v.y / sinTheta, -1.f, 1.f));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> cos_phi(const T &v) noexcept {
    scalar_t<T> sinTheta = sin_theta(v);
    return select(sinTheta == 0.f, 1.f, clamp(v.x / sinTheta, -1.f, 1.f));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> sin_phi_2(const T &v) {
    scalar_t<T> sinTheta2 = sin_theta_2(v);
    return select(sinTheta2 == 0.f, 0.f, clamp(sqr(v.y) / sinTheta2, 0.f, 1.f));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> cos_phi_2(const T &v) {
    scalar_t<T> sinTheta2 = sin_theta_2(v);
    return select(sinTheta2 == 0.f, 1.f, clamp(sqr(v.x) / sinTheta2, 0.f, 1.f));
}

template<typename T>
[[nodiscard]] ray_t<T> spawn_ray(T pos, T normal, T dir) {
    normal *= select(dot(normal, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(pos, normal);
    return make_ray(org, dir);
}

template<typename T, typename U>
[[nodiscard]] ray_t<T> spawn_ray(T pos, T normal, T dir, U t_max) {
    normal *= select(dot(normal, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(pos, normal);
    return make_ray(org, dir, t_max);
}

template<typename T>
[[nodiscard]] ray_t<T> spawn_ray_to(T p_start, T n_start, T p_target) {
    T dir = p_target - p_start;
    n_start *= select(dot(n_start, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(p_start, n_start);
    return make_ray(org, dir, 1 - ShadowEpsilon);
}

template<typename T>
[[nodiscard]] ray_t<T> spawn_ray_to(T p_start, T n_start, T p_target, T n_target) {
    T dir = p_target - p_start;
    n_target *= select(dot(n_target, -dir) > 0, 1.f, -1.f);
    p_target = offset_ray_origin(p_target, n_target);
    n_start *= select(dot(n_start, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(p_start, n_start);
    return make_ray(org, dir, 1 - ShadowEpsilon);
}

template<EPort p = D>
[[nodiscard]] oc_float3<p> spherical_direction(oc_float<p> sin_theta, oc_float<p> cos_theta,
                                               oc_float<p> sin_phi, oc_float<p> cos_phi) {
    return make_float3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta);
}

template<EPort p = D>
[[nodiscard]] oc_float3<p> spherical_direction(oc_float<p> sin_theta, oc_float<p> cos_theta,
                                               oc_float<p> phi) {
    return make_float3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}

template<EPort p = D>
[[nodiscard]] oc_float3<p> spherical_direction(oc_float<p> theta, oc_float<p> phi) {
    return spherical_direction(sin(theta), cos(theta), phi);
}
template<EPort p = D>
[[nodiscard]] oc_float3<p> spherical_direction(oc_float<p> sin_theta, oc_float<p> cos_theta, oc_float<p> phi,
                                               const oc_float3<p> &x, const oc_float3<p> &y,
                                               const oc_float3<p> &z) {
    return sin_theta * cos(phi) * x + sin_theta * sin(phi) * y + cos_theta * z;
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] auto spherical_theta(const T &v) {
    return safe_acos(v.z);
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] auto spherical_phi(const T &v) {
    auto p = atan2(v.y, v.x);
    return select((p < 0), (p + 2 * Pi), p);
}

template<typename T>
requires is_vector3_expr_v<T>
struct Frame {
public:
    using vec_ty = T;
    vec_ty x, y, z;

public:
    Frame() = default;
    Frame(const T &x, const T &y, const T &z)
        : x(x), y(y), z(z) {}

    explicit Frame(const T &normal)
        : z(normal) {
        ocarina::coordinate_system(z, x, y);
    }

    void set(const vec_ty &x_, const vec_ty &y_, const vec_ty &z_) {
        x = x_;
        y = y_;
        z = z_;
    }

    template<typename TVec>
    [[nodiscard]] auto to_local(const TVec &world_v) const noexcept {
        return make_float3(dot(world_v, x), dot(world_v, y), dot(world_v, z));
    }

    template<typename TVec>
    [[nodiscard]] auto to_world(const TVec &local_v) const noexcept {
        return x * local_v.x + y * local_v.y + z * local_v.z;
    }

    template<typename U>
    [[nodiscard]] auto operator==(const Frame<U> &frame) const noexcept {
        return frame.x == x && frame.y == y && frame.z == z;
    }

    template<typename U>
    [[nodiscard]] auto operator!=(const Frame<U> &frame) const {
        return !operator==(frame);
    }
};

template<typename T, typename U>
requires is_vector3_expr_v<T> && is_vector3_expr_v<U>
[[nodiscard]] condition_t<bool, T, U> same_hemisphere(const T &w1, const U &w2) noexcept {
    return w1.z * w2.z > 0;
}

}// namespace geometry
}// namespace vision

namespace vision {
using namespace ocarina;
inline namespace geometry {
struct Triangle {
    uint i, j, k;
    Triangle(uint i, uint j, uint k) : i(i), j(j), k(k) {}
    Triangle() = default;
};
}// namespace geometry
}// namespace vision
OC_STRUCT(vision::Triangle, i, j, k){};

namespace vision {
using namespace ocarina;
inline namespace geometry {
struct Vertex {
public:
    //todo compress
    array<float, 3> pos;
    array<float, 3> n;
    array<float, 2> uv;
    array<float, 2> uv2;

public:
    Vertex() = default;
    Vertex(float3 p, float3 n, float2 uv, float2 uv2 = make_float2(0.f))
        : pos{p.x, p.y, p.z},
          n{n.x, n.y, n.z},
          uv{uv.x, uv.y},
          uv2{uv2.x, uv2.y} {}

    [[nodiscard]] auto position() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }

    [[nodiscard]] auto normal() const noexcept {
        return make_float3(n[0], n[1], n[2]);
    }

    [[nodiscard]] auto tex_coord() const noexcept {
        return make_float2(uv[0], uv[1]);
    }

    [[nodiscard]] auto lightmap_uv() const noexcept {
        return make_float2(uv2[0], uv2[1]);
    }

#define VS_ATTR_OFFSET(attr)                               \
    [[nodiscard]] static size_t attr##_offset() noexcept { \
        return OC_OFFSET_OF(Vertex, attr);                 \
    }
    VS_ATTR_OFFSET(pos)
    VS_ATTR_OFFSET(n)
    VS_ATTR_OFFSET(uv)
    VS_ATTR_OFFSET(uv2)
#undef VS_ATTR_STRIDE_OFFSET
};
}// namespace geometry
}// namespace vision

OC_STRUCT(vision::Vertex, pos, n, uv, uv2){
    [[nodiscard]] auto position() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }

    [[nodiscard]] auto normal() const noexcept {
        return make_float3(n[0], n[1], n[2]);
    }

    [[nodiscard]] auto tex_coord() const noexcept {
        return make_float2(uv[0], uv[1]);
    }

    [[nodiscard]] auto lightmap_uv() const noexcept {
        return make_float2(uv2[0], uv2[1]);
    }
};
