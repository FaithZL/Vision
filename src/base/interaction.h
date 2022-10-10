//
// Created by Zero on 03/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "math/geometry.h"

namespace vision {

using namespace ocarina;

template<typename T>
requires is_vector3_expr_v<T>
struct UVN : Frame<T> {
public:
    using vec_ty = T;

public:
    void set_frame(Frame<T> frame) {
        this->x = frame.x;
        this->y = frame.y;
        this->z = frame.z;
    }
    [[nodiscard]] vec_ty dp_du() const noexcept { return this->x; }
    [[nodiscard]] vec_ty dp_dv() const noexcept { return this->y; }
    [[nodiscard]] vec_ty normal() const noexcept { return this->z; }
    [[nodiscard]] boolean_t<T> valid() const noexcept { return any(normal() != 0.f); }
};

template<typename T>
requires is_vector3_expr_v<T>
struct Interaction {
public:
    using vec_ty = T;
    using scalar_ty = scalar_t<vec_ty>;

public:
    vec_ty pos;
    vec_ty wo;
    scalar_ty time;
    UVN<vec_ty> g_uvn;

public:
    Interaction() = default;
    [[nodiscard]] boolean_t<T> on_surface() const noexcept { return g_uvn.valid(); }
    [[nodiscard]] ray_t<T> spawn_ray(const vec_ty &dir) const noexcept {
        return vision::spawn_ray(pos, g_uvn.normal(), dir);
    }
    [[nodiscard]] ray_t<T> spawn_ray_to(const vec_ty &p) const noexcept {
        return vision::spawn_ray_to(pos, g_uvn, p);
    }
    [[nodiscard]] ray_t<T> spawn_ray_to(const Interaction<T> &it) const noexcept {
        return vision::spawn_ray_to(pos, g_uvn.normal(), it.pos, it.g_uvn.normal());
    }
};

}// namespace vision