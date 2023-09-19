//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;
struct RSVSample {
    uint light_index{};
    uint prim_id{};
    float2 u;
    float p_hat;
    float pdf;
    array<float, 3> pos;
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_pos(float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::RSVSample, light_index, prim_id, u, p_hat, pdf, pos) {
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_pos(Float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
};
// clang-format on

namespace vision {
using namespace ocarina;
struct SurfaceData {
    Hit hit{};
    float4 normal_t;
};

}// namespace vision

// clang-format off
OC_STRUCT(vision::SurfaceData, hit, normal_t) {
    void set_normal(const Float3 &n) {
        normal_t = make_float4(n, normal_t.w);
    }
    [[nodiscard]] Float3 normal() const noexcept { return normal_t.xyz();}
    void set_t_max(const Float &t) { normal_t.w = t; }
    [[nodiscard]] Float t_max() const noexcept { return normal_t.w;}
};
// clang-format on

namespace vision {
using OCSurfaceData = Var<SurfaceData>;
using OCRSVSample = Var<RSVSample>;
}// namespace vision

namespace vision {
using namespace ocarina;

struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_uint<p> M{};
    oc_float<p> W{};
    RSVSample sample{};

public:
    void update(oc_float<p> u, oc_float<p> weight, RSVSample v) {
        weight_sum += weight;
        M += 1;
        sample = ocarina::select(u < (weight / M), v, sample);
    }
    void update(oc_float<p> u, RSVSample v) {
        oc_float<p> weight = ocarina::select(v.pdf == 0, 0.f, v.p_hat / v.pdf);
        update(u, weight, v);
    }
    void update_W() noexcept {
        auto denominator = M * sample.p_hat;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
    }
    [[nodiscard]] auto compute_weight_sum() const noexcept {
        return sample.p_hat * W * M;
    }
    void reset_W() noexcept { W = 0.f; }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, M, W, sample) {
    static constexpr EPort p = D;
    void update(oc_float<p> u, oc_float<p> weight, vision::OCRSVSample v) {
        weight_sum += weight;
        M += 1;
        sample = select(u < (weight / weight_sum), v, sample);
    }
    void update(oc_float<p> u, vision::OCRSVSample v) {
        oc_float<p> weight = ocarina::select(v.pdf == 0, 0.f, v.p_hat / v.pdf);
        update(u, weight, v);
    }
    void update_W() noexcept {
        auto denominator = M * sample.p_hat;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
    }
    [[nodiscard]] auto compute_weight_sum() const noexcept {
        return sample.p_hat * W * M;
    }
    void reset_W() noexcept { W = 0.f; }
};

namespace vision {
using namespace ocarina;
using OCReservoir = Var<Reservoir>;
[[nodiscard]] inline OCReservoir combine_reservoir(const OCReservoir &r0,
                                                   const OCReservoir &r1,
                                                   const Float &u) noexcept {
    OCReservoir ret;
    ret = r0;
    ret->update(u, r1->compute_weight_sum(), r1.sample);
    ret.M = r0.M + r1.M;
    ret->update_W();
    return ret;
}
}// namespace vision