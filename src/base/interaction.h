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
    [[nodiscard]] boolean_t<T> valid() const noexcept { return nonzero(normal()); }
};

template<EPort p = D>
struct Interaction {
public:
    oc_float3<p> pos;
    oc_float3<p> wo;
    oc_float<p> time;
    UVN<oc_float3<p>> g_uvn;

public:
    Interaction() = default;
    [[nodiscard]] oc_bool<p> on_surface() const noexcept { return g_uvn.valid(); }
    [[nodiscard]] var_t<Ray, p> spawn_ray(const Float3 &dir) const noexcept {
        return vision::spawn_ray(pos, g_uvn.normal(), dir);
    }
    [[nodiscard]] var_t<Ray, p> spawn_ray_to(const Float3 &pos) const noexcept {
        return vision::spawn_ray_to(pos, g_uvn.normal(), pos);
    }
    [[nodiscard]] var_t<Ray, p> spawn_ray_to(const Interaction &it) const noexcept {
        return vision::spawn_ray_to(pos, g_uvn.normal(), it.pos, it.g_uvn.normal());
    }
};

template<EPort p = D>
struct SurfaceInteraction : public Interaction<p> {
    oc_float2<p> uv;
    UVN<oc_float3<p>> s_uvn;
    oc_float<p> PDF_pos{-1.f};
    oc_float<p> prim_area{0.f};
    oc_uint<p> light_id{InvalidUI32};
    oc_uint<p> mat_id{InvalidUI32};
    [[nodiscard]] oc_bool<p> has_emission() const noexcept { return light_id != InvalidUI32; }
    [[nodiscard]] oc_bool<p> has_material() const noexcept { return mat_id != InvalidUI32; }
};


}// namespace vision