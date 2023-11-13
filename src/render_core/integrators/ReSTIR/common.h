//
// Created by Zero on 2023/11/7.
//

#pragma once

#include "core/stl.h"
#include "dsl/common.h"
#include "descriptions/parameter_set.h"
#include "math/base.h"

namespace vision {

using namespace ocarina;

struct SpatialResamplingParam {
public:
    float dot_threshold{};
    float depth_threshold{};
    float sampling_radius{};
    uint iterate_num{};
    bool open{};

public:
    SpatialResamplingParam() = default;
    explicit SpatialResamplingParam(const ParameterSet &ps)
        : dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.01f)),
          sampling_radius(ps["radius"].as_float(3.f)),
          iterate_num(ps["iterate_num"].as_uint(5)),
          open{ps["open"].as_bool(true)} {}
};

struct TemporalResamplingParam {
public:
    uint history_limit{};
    float sampling_radius{};
    float2 motion_vec_threshold{};
    float dot_threshold{};
    float depth_threshold{};
    bool open{};

public:
    TemporalResamplingParam() = default;
    TemporalResamplingParam(const ParameterSet &ps, uint2 res)
        : history_limit(ps["history"].as_uint(20)),
          sampling_radius(ps["radius"].as_float(2.f)),
          motion_vec_threshold(ps["motion_vec"].as_float2(make_float2(0.15f)) * make_float2(res)),
          dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.1f)),
          open{ps["open"].as_bool(true)} {}
};

}// namespace vision

namespace vision {
using namespace ocarina;
struct SurfaceData {
    Hit hit{};
    float4 normal_t;
    uint mat_id{};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision::SurfaceData, hit, normal_t, mat_id) {
    void set_normal(const Float3 &n) {
        normal_t = make_float4(n, normal_t.w);
    }
    [[nodiscard]] Float3 normal() const noexcept { return normal_t.xyz();}
    void set_t_max(const Float &t) { normal_t.w = t; }
    [[nodiscard]] Bool valid() const { return t_max() > 0.f; }
    [[nodiscard]] Float t_max() const noexcept { return normal_t.w;}
};
// clang-format on

namespace vision {
using OCSurfaceData = Var<SurfaceData>;
[[nodiscard]] inline Bool is_neighbor(const OCSurfaceData &cur_surface,
                                      const OCSurfaceData &another_surface,
                                      float dot_threshold, float depth_threshold) noexcept {
    Bool cond0 = abs_dot(cur_surface->normal(), another_surface->normal()) > dot_threshold;
    Bool cond1 = (abs(cur_surface->t_max() - another_surface->t_max()) / cur_surface->t_max()) < depth_threshold;
    return cond0 && cond1 && (cur_surface.mat_id == another_surface.mat_id);
}
}// namespace vision
