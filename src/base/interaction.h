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
    [[nodiscard]] boolean_t<T> on_surface() const noexcept { return g_uvn.valid(); }
};

}// namespace vision