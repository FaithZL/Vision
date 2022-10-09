//
// Created by Zero on 19/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;
inline namespace geometry {
struct Triangle {
    uint i, j, k;
};
}
}// namespace vision::geometry
OC_STRUCT(vision::Triangle, i, j, k){};

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
    return select(sinTheta == 0, 1.f, clamp(v.y / sinTheta, -1, 1));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] scalar_t<T> cos_phi(const T &v) noexcept {
    scalar_t<T> sinTheta = sin_theta(v);
    if (sinTheta == 0.f) {
        return 1.f;
    }
    return clamp(v.x / sinTheta, -1.f, 1.f);
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

inline Ray spawn_ray(float3 pos, float3 normal, float3 dir) {
    normal *= select(dot(normal, dir) > 0, 1.f, -1.f);
    float3 org = offset_ray_origin(pos, normal);
    return Ray(pos, dir);
}

inline Var<Ray> spawn_ray(Float3 pos, Float3 normal, Float3 dir) {
    normal *= select(dot(normal, dir) > 0, 1.f, -1.f);
    Float3 org = offset_ray_origin(pos, normal);
    return make_ray(pos, dir);
}

inline Ray spawn_ray_to(float3 p_start, float3 n_start, float3 p_target) {
    float3 dir = p_target - p_start;
    float3 org = offset_ray_origin(p_start, n_start);
    n_start *= select(dot(n_start, dir) > 0, 1.f, -1.f);
    return Ray(org, dir, 1 - ShadowEpsilon);
}

inline Var<Ray> spawn_ray_to(Float3 p_start, Float3 n_start, Float3 p_target) {
    Float3 dir = p_target - p_start;
    Float3 org = offset_ray_origin(p_start, n_start);
    n_start *= select(dot(n_start, dir) > 0, 1.f, -1.f);
    return make_ray(org, dir, 1 - ShadowEpsilon);
}

template<typename T>
requires is_vector3_expr_v<T>
struct Frame {
public:
    using vec_ty = T;
    vec_ty x, y, z;

public:
    Frame(const T &x, const T &y, const T &z)
        : x(x), y(y), z(z) {}

    explicit Frame(const T &normal)
        : z(normal) {
        ocarina::coordinate_system(z, x, y);
    }

    template<typename TVec>
    [[nodiscard]] auto to_local(const TVec &world_v) const noexcept {
        return make_float3(dot(world_v, x), dot(world_v, y), dot(world_v, z));
    }

    template<typename TVec>
    [[nodiscard]] auto to_world(const TVec & local_v) const noexcept {
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
}// namespace geometry
}// namespace vision

namespace vision {
using namespace ocarina;
inline namespace geometry {
struct Vertex {
    //todo compress
    float3 position;
    float3 normal;
    float3 tex_coord;
};
}
}// namespace vision::geometry
OC_STRUCT(vision::Vertex, position, normal, tex_coord){};

namespace vision {
using namespace ocarina;
inline namespace geometry {
struct Mesh {
    uint vertex_offset;
    uint triangle_offset;
    uint vertex_count;
    uint triangle_count;
    uint distribute_idx;
    uint material_idx;
    uint light_idx;
};
}
}// namespace vision::geometry
OC_STRUCT(vision::Mesh, vertex_offset, triangle_offset, vertex_count,
          triangle_count, distribute_idx, material_idx, light_idx){};