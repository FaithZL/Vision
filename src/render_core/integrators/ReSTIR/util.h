//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"
#include "base/sampler.h"
#include "common.h"

namespace vision::ReSTIRDirect {
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
}// namespace vision::ReSTIRDirect

// clang-format off
OC_STRUCT(vision::ReSTIRDirect::RSVSample, light_index, prim_id, u, p_hat, pdf, pos) {
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
using DIRSVSample = Var<ReSTIRDirect::RSVSample>;
}// namespace vision

namespace vision {
using namespace ocarina;

namespace ReSTIRDirect {
struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_uint<p> M{};
    oc_float<p> W{};
    RSVSample sample{};

public:
    bool update(oc_float<p> u, oc_float<p> weight, RSVSample v) noexcept {
        weight_sum += weight;
        M += 1;
        bool ret = u < (weight / weight_sum);
        sample = ocarina::select(ret, v, sample);
        return ret;
    }
    bool update(oc_float<p> u, vision::ReSTIRDirect::Reservoir rsv) noexcept {
        auto temp_M = M;
        bool ret = update(u, rsv.compute_weight_sum(), rsv.sample);
        M = temp_M + rsv.M;
        return ret;
    }
    bool update(oc_float<p> u, RSVSample v) noexcept {
        oc_float<p> weight = ocarina::select(v.pdf == 0, 0.f, v.p_hat / v.pdf);
        return update(u, weight, v);
    }
    void truncation(oc_uint<p> limit) noexcept {
        oc_float<p> factor = cast<float>(limit) / M;
        M = ocarina::select(factor < 1.f, limit, M);
        weight_sum = ocarina::select(factor < 1.f, weight_sum * factor, weight_sum);
    }
    void process_occluded(oc_bool<p> occluded) noexcept {
        W = ocarina::select(occluded, 0.f, W);
        weight_sum = ocarina::select(occluded, 0.f, weight_sum);
    }
    void update_W() noexcept {
        auto denominator = M * sample.p_hat;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
    }
    void update_W_MIS(const float &p_sum) noexcept {
        float mis = sample.p_hat / p_sum;
        W = ocarina::select(sample.p_hat == 0.f, 0.f, mis * weight_sum / sample.p_hat);
    }
    [[nodiscard]] oc_float<p> compute_weight_sum() const noexcept {
        return sample.p_hat * W * M;
    }
};
}// namespace ReSTIRDirect

}// namespace vision

OC_STRUCT(vision::ReSTIRDirect::Reservoir, weight_sum, M, W, sample) {
    static constexpr EPort p = D;
    Bool update(oc_float<p> u, oc_float<p> weight, vision::DIRSVSample v) noexcept {
        weight_sum += weight;
        M += 1;
        Bool ret = u < (weight / weight_sum);
        sample = select(ret, v, sample);
        return ret;
    }
    Bool update(oc_float<p> u, vision::DIRSVSample v) noexcept {
        oc_float<p> weight = ocarina::select(v.pdf == 0, 0.f, v.p_hat / v.pdf);
        return update(u, weight, v);
    }
    Bool update(oc_float<p> u, Var<vision::ReSTIRDirect::Reservoir> rsv) noexcept {
        auto temp_M = M;
        Bool ret = update(u, rsv->compute_weight_sum(), rsv.sample);
        M = temp_M + rsv.M;
        return ret;
    }
    void truncation(oc_uint<p> limit) noexcept {
        oc_float<p> factor = cast<float>(limit) / M;
        M = ocarina::select(factor < 1.f, limit, M);
        weight_sum = ocarina::select(factor < 1.f, weight_sum * factor, weight_sum);
    }
    void process_occluded(oc_bool<p> occluded) noexcept {
        W = ocarina::select(occluded, 0.f, W);
        weight_sum = ocarina::select(occluded, 0.f, weight_sum);
    }
    void update_W() noexcept {
        Float denominator = M * sample.p_hat;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
    }
    void update_W_MIS(const Float &p_sum) noexcept {
        Float mis = sample.p_hat / p_sum;
        W = ocarina::select(sample.p_hat == 0.f, 0.f, mis * weight_sum / sample.p_hat);
    }
    [[nodiscard]] oc_float<p> compute_weight_sum() const noexcept {
        return sample.p_hat * W * M;
    }
};

namespace vision {
using namespace ReSTIRDirect;
using namespace ocarina;
using DIReservoir = Var<ReSTIRDirect::Reservoir>;
}// namespace vision