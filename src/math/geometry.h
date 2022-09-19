//
// Created by Zero on 19/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/struct.h"

namespace vision {
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
[[nodiscard]] auto abs_cos_theta(const T &v) noexcept{ return ocarina::abs(v.z); }

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] auto sin_theta_2(const T &v)noexcept { return 1.0f - cos_theta_2(v); }

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] auto sin_theta(const T &v)noexcept {
    auto temp = sin_theta_2(v);
    return select(temp <= 0.f, 0.f, sqrt(temp));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] auto tan_theta(const T &v) noexcept {
    auto sin_theta2 = 1 - cos_theta_2(v);
    if (sin_theta2 <= 0.0f) {
        return 0.0f;
    }
    return std::sqrt(sin_theta2) / cos_theta(v);
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

    [[nodiscard]] vec_ty to_local(vec_ty world_v) const noexcept {
        return vec_ty(dot(world_v, x), dot(world_v, y), dot(world_v, z));
    }

    [[nodiscard]] vec_ty to_world(vec_ty local_v) const noexcept {
        return x * local_v.x + y * local_v.y + z * local_v.z;
    }
};
}// namespace geometry
}// namespace vision

namespace vision {
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