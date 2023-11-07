//
// Created by Zero on 2023/11/7.
//

#include "core/stl.h"
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

public:
    SpatialResamplingParam() = default;
    explicit SpatialResamplingParam(const ParameterSet &ps)
        : dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.01f)),
          sampling_radius(ps["radius"].as_float(3.f)),
          iterate_num(ps["iterate_num"].as_uint(5)) {}
};

struct TemporalResamplingParam {
public:
    uint history_limit{};
    float sampling_radius{};
    float2 motion_vec_threshold{};
    float dot_threshold{};
    float depth_threshold{};

public:
    TemporalResamplingParam() = default;
    TemporalResamplingParam(const ParameterSet &ps, uint2 res)
        : history_limit(ps["history"].as_uint(10)),
          sampling_radius(ps["radius"].as_float(2.f)),
          motion_vec_threshold(ps["motion_vec"].as_float2(make_float2(0.15f)) * make_float2(res)),
          dot_threshold(cosf(radians(ps["theta"].as_float(5)))),
          depth_threshold(ps["depth"].as_float(0.1f)) {}
};

}
