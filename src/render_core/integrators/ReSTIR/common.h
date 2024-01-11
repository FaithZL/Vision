//
// Created by Zero on 2023/11/7.
//

#pragma once

#include "core/stl.h"
#include "dsl/dsl.h"
#include "descriptions/parameter_set.h"
#include "math/base.h"

namespace vision {

using namespace ocarina;

struct SpatialResamplingParam {
public:
    float dot_threshold{};
    float depth_threshold{};
    float sampling_radius{};
    uint sample_num{};
    bool open{};

public:
    SpatialResamplingParam() = default;
    explicit SpatialResamplingParam(const ParameterSet &ps)
        : dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.3f)),
          sampling_radius(ps["radius"].as_float(30)),
          sample_num(ps["sample_num"].as_uint(1)),
          open{ps["open"].as_bool(true)} {
        sample_num = open ? sample_num : 1;
    }
};

struct TemporalResamplingParam {
public:
    uint limit{};
    float sampling_radius{};
    float dot_threshold{};
    float depth_threshold{};
    bool open{};
    bool mis{};

public:
    TemporalResamplingParam() = default;
    explicit TemporalResamplingParam(const ParameterSet &ps)
        : limit(ps["history_limit"].as_uint(5)),
          sampling_radius(ps["radius"].as_float(4)),
          dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.5)),
          open{ps["open"].as_bool(true)},
          mis{ps["mis"].as_bool(false)} {}
};

}// namespace vision

namespace vision {
using namespace ocarina;
struct SurfaceData {
    Hit hit{};
    float4 normal_t{};
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
