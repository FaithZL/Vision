//
// Created by Zero on 15/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "core/concepts.h"
#include "box.h"

namespace vision {
using namespace ocarina;

inline namespace transform {

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] constexpr auto translation(const T &t) noexcept {
    return make_float4x4(
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        t.x, t.y, t.z, 1.f);
}

template<typename T>
requires is_scalar_expr_v<T>
[[nodiscard]] constexpr auto translation(const T &x, const T &y, const T &z) noexcept {
    return translation(make_float3(x, y, z));
}

template<typename T>
requires is_vector3_expr_v<T>
[[nodiscard]] constexpr auto scale(const T &s) noexcept {
    return make_float4x4(
        s.x, 0.f, 0.f, 0.f,
        0.f, s.y, 0.f, 0.f,
        0.f, 0.f, s.z, 0.f,
        0.f, 0.f, 0.f, 1.f);
}

template<typename T>
requires is_scalar_expr_v<T>
[[nodiscard]] constexpr auto scale(const T &x, const T &y, const T &z) noexcept { return scale(make_float3(x, y, z)); }

template<typename T>
requires is_scalar_expr_v<T>
[[nodiscard]] constexpr auto scale(const T &s) noexcept { return scale(make_float3(s)); }

template<typename F, typename B>
[[nodiscard]] inline auto perspective(F fov_y, const F &z_near, const F &z_far, B radian = false) noexcept {
    fov_y = select(radian, fov_y, radians(fov_y));
    F inv_tan = 1 / tan(fov_y / 2.f);
    auto mat = make_float4x4(
        inv_tan, 0, 0, 0,
        0, inv_tan, 0, 0,
        0, 0, z_far / (z_far - z_near), 1,
        0, 0, -z_far * z_near / (z_far - z_near), 0);
    return mat;
}

template<typename V, typename F, typename B>
[[nodiscard]] inline auto rotation(const V &axis, F angle, B radian = false) noexcept {
    angle = ocarina::select(radian, angle, radians(angle));

    F c = cos(angle);
    F s = sin(angle);
    V a = normalize(axis);
    vec3_t<F> t = (1.0f - c) * a;

    auto mat = make_float4x4(
        c + t.x * a.x, t.x * a.y + s * a.z, t.x * a.z - s * a.y, 0.0f,
        t.y * a.x - s * a.z, c + t.y * a.y, t.y * a.z + s * a.x, 0.0f,
        t.z * a.x + s * a.y, t.z * a.y - s * a.x, c + t.z * a.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return mat;
}

template<typename F, typename B>
[[nodiscard]] inline auto rotation_x(F angle, B radian = false) noexcept {
    return rotation(make_float3(1, 0, 0), angle, radian);
}

template<typename F, typename B>
[[nodiscard]] inline auto rotation_y(F angle, B radian = false) noexcept {
    return rotation(make_float3(0, 1, 0), angle, radian);
}

template<typename F, typename B>
[[nodiscard]] inline auto rotation_z(F angle, B radian = false) noexcept {
    return rotation(make_float3(0, 0, 1), angle, radian);
}

template<typename F3, typename F4>
[[nodiscard]] inline auto TRS(F3 t, F4 r, F3 s) {
    auto T = translation(t);
    auto R = rotation(make_float3(r), r.w, false);
    auto S = scale(s);
    return T * R * S;
}

template<typename F3>
[[nodiscard]] inline auto look_at(F3 eye, F3 target_pos, F3 up) noexcept {
    F3 fwd = normalize(target_pos - eye);
    F3 right = normalize(cross(up, fwd));
    up = normalize(cross(fwd, right));
    auto mat = make_float4x4(
        right.x, right.y, right.z, 0.f,
        up.x, up.y, up.z, 0.f,
        fwd.x, fwd.y, fwd.z, 0.f,
        eye.x, eye.y, eye.z, 1.f);
    return mat;
}

template<typename M, typename T>
requires is_matrix4_v<expr_value_t<M>> && is_vector3_expr_v<T>
[[nodiscard]] auto transform_vector(const M &m, const T &vec) noexcept {
    auto mat3x3 = make_float3x3(m);
    return mat3x3 * vec;
}

template<typename M, typename T>
requires is_matrix4_v<expr_value_t<M>> && is_vector3_expr_v<T>
[[nodiscard]] auto transform_point(const M &m, const T &point) noexcept {
    auto homo_point = make_float4(point, 1.f);
    return make_float3(m * homo_point);
}

template<typename M, typename T>
requires is_matrix4_v<expr_value_t<M>> && is_vector3_expr_v<T>
[[nodiscard]] auto transform_normal(const M &m, const T &normal) noexcept {
    return make_float3(transpose(inverse(m)) * make_float4(normal, 0.f));
}

template<typename M, typename TRay>
requires std::is_same_v<expr_value_t<TRay>, Ray>
[[nodiscard]] auto transform_ray(const M &m, TRay ray) noexcept {
    ray.update_origin(transform_point(m, ray.origin()));
    ray.update_direction(transform_vector(m, ray.direction()));
    return ray;
}

[[nodiscard]] inline Box3f transform_box(float4x4 mat, const Box3f &b) noexcept {
    float3 minPoint = make_float3(mat[3][0], mat[3][1], mat[3][2]);
    float3 maxPoint = minPoint;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            float e = mat[j][i];
            float p1 = e * b.lower[j];
            float p2 = e * b.upper[j];
            if (p1 > p2) {
                minPoint[i] += p2;
                maxPoint[i] += p1;
            } else {
                minPoint[i] += p1;
                maxPoint[i] += p2;
            }
        }
    }
    return {minPoint, maxPoint};
}

}// namespace transform

template<typename TMat>
requires is_matrix4_v<expr_value_t<TMat>>
struct Transform {
private:
    TMat _mat;

public:
    Transform() = default;
    explicit Transform(const TMat &mat) : _mat(mat) {}
    [[nodiscard]] TMat mat4x4() const noexcept { return _mat; }
    [[nodiscard]] auto inv_mat4x4() const noexcept { return inverse(_mat); }
    [[nodiscard]] auto mat3x3() const noexcept { return make_float3x3(_mat); }
    [[nodiscard]] auto inv_mat3x3() const noexcept { return inverse(mat3x3()); }
    template<typename M>
    [[nodiscard]] auto operator*(const Transform<M> &other) const noexcept { return Transform(_mat * other._mat); }
    template<typename T>
    [[nodiscard]] auto apply_vector(const T &vec) noexcept { return transform_vector(_mat, vec); }
    template<typename T>
    [[nodiscard]] auto apply_point(const T &point) noexcept { return transform_point(_mat, point); }
    template<typename M, typename T>
    [[nodiscard]] auto apply_normal(const T &normal) noexcept { return transform_normal(_mat, normal); }
    template<typename TRay>
    [[nodiscard]] auto apply_ray(TRay &&ray) noexcept { return transform_ray(_mat, OC_FORWARD(ray)); }

public:
#define VISION_MAKE_TRANSFORM_CREATOR(func_name)                   \
    template<typename... Args>                                     \
    [[nodiscard]] static auto func_name(Args &&...args) noexcept { \
        return Transform(vision::func_name(OC_FORWARD(args)...));  \
    }

    VISION_MAKE_TRANSFORM_CREATOR(translation)
    VISION_MAKE_TRANSFORM_CREATOR(scale)
    VISION_MAKE_TRANSFORM_CREATOR(rotation)
    VISION_MAKE_TRANSFORM_CREATOR(rotation_x)
    VISION_MAKE_TRANSFORM_CREATOR(rotation_y)
    VISION_MAKE_TRANSFORM_CREATOR(rotation_z)
    VISION_MAKE_TRANSFORM_CREATOR(perspective)
    VISION_MAKE_TRANSFORM_CREATOR(look_at)
    VISION_MAKE_TRANSFORM_CREATOR(TRS)

#undef VISION_MAKE_TRANSFORM_CREATOR
};

}// namespace vision