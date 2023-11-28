//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/dsl.h"
#include "base/sampler.h"
#include "common.h"

namespace vision::ReSTIRDirect {
struct RSVSample {
    uint light_index{};
    uint prim_id{};
    float2 u;
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
OC_STRUCT(vision::ReSTIRDirect::RSVSample, light_index, prim_id, u, pos) {
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
    oc_float<p> M{};
    oc_float<p> p_sum{};
    oc_float<p> W{};
    RSVSample sample{};
};
}// namespace ReSTIRDirect

}// namespace vision

OC_STRUCT(vision::ReSTIRDirect::Reservoir, weight_sum, M, p_sum, W, sample) {
    static constexpr EPort p = D;
    Bool update(oc_float<p> u, oc_float<p> p_hat, oc_float<p> pdf, vision::DIRSVSample v) noexcept {
        oc_float<p> weight = p_hat / pdf;
        weight_sum += weight;
        p_sum += pdf;
        M += 1;
        Bool ret = u < (weight / weight_sum);
        sample = select(ret, v, sample);
        return ret;
    }
    Bool update(oc_float<p> u, Var<vision::ReSTIRDirect::Reservoir> rsv, oc_float<p> p_hat) noexcept {
        oc_float<p> weight = rsv->compute_weight_sum(p_hat);
        weight_sum += weight;
        M += rsv.M;
        p_sum += rsv.p_sum;
        Bool ret = u < (weight / weight_sum);
        sample = select(ret, rsv.sample, sample);
        return ret;
    }
    void truncation(oc_uint<p> limit) noexcept {
        oc_float<p> factor = cast<float>(limit) / M;
        M = ocarina::select(factor < 1.f, cast<float>(limit), M);
        weight_sum = ocarina::select(factor < 1.f, weight_sum * factor, weight_sum);
        p_sum = ocarina::select(factor < 1.f, p_sum * factor, p_sum);
    }
    void process_occluded(oc_bool<p> occluded) noexcept {
        W = ocarina::select(occluded, 0.f, W);
        weight_sum = ocarina::select(occluded, 0.f, weight_sum);
    }
    void update_W(oc_float<p> p_hat) noexcept {
        Float denominator = M * p_hat;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
    }
    void update_W_MIS(const Float &p_hat, const Float &pdf) noexcept {
        Float mis = 1.f / M;
        Float mis2 = pdf / p_sum;
//        $condition_info("{} {} {}", 1.f / M, pdf / p_sum, weight_sum);
        W = ocarina::select(p_hat == 0.f, 0.f, mis2 * weight_sum / p_hat);
    }
    [[nodiscard]] oc_float<p> compute_weight_sum(oc_float<p> p_hat) const noexcept {
        return p_hat * W * M;
    }
};

namespace vision {
using namespace ReSTIRDirect;
using namespace ocarina;
using DIReservoir = Var<ReSTIRDirect::Reservoir>;
}// namespace vision