//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/dsl.h"
#include "base/sampler.h"
#include "base/illumination/light.h"
#include "common.h"

namespace vision::ReSTIRDirect {
struct RSVSample {
    uint light_index{};
    uint prim_id{};
    float2 u{};
    float p_hat{};
    array<float, 3> pos{};
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
OC_STRUCT(vision::ReSTIRDirect::RSVSample, light_index,prim_id, u, p_hat, pos) {
    void init() noexcept {
        light_index = InvalidUI32;
    }
    [[nodiscard]] Bool valid() const noexcept {
        return light_index != InvalidUI32;
    }
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_light_surface_point(const vision::LightSurfacePoint &lsp) noexcept {
        u = lsp.uv;
        light_index = lsp.light_index;
        prim_id = lsp.prim_id;
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
    oc_float<p> canonical_weight{};
    RSVSample sample{};
    template<EPort p_ = D>
    [[nodiscard]] static oc_float<p_> calculate_weight(oc_float<p_> mis_weight, oc_float<p_> p_hat,
                                                       oc_float<p_> pdf) noexcept {
        return mis_weight * p_hat / pdf;
    }
};
}// namespace ReSTIRDirect

}// namespace vision

OC_STRUCT(vision::ReSTIRDirect::Reservoir, weight_sum, M, W, canonical_weight, sample) {
    static constexpr EPort p = D;
    void init() noexcept {
        sample->init();
    }
    [[nodiscard]] Bool valid() const noexcept {
        return sample->valid();
    }
    Bool update(oc_float<p> u, vision::DIRSVSample v, oc_float<p> weight, oc_uint<p> sample_M = 1u) noexcept {
        weight_sum += weight;
        M += sample_M;
        Bool ret = u * weight_sum < weight;
        sample = select(ret, v, sample);
        return ret;
    }
    Bool update(oc_float<p> u, oc_float<p> p_hat, oc_float<p> pdf, vision::DIRSVSample v) noexcept {
        oc_float<p> weight = p_hat / pdf;
        weight = ocarina::select(pdf == 0, 0.f, weight);
        return update(u, v, weight, 1);
    }
    Bool combine(oc_float<p> u, Var<vision::ReSTIRDirect::Reservoir> rsv, oc_float<p> p_hat) noexcept {
        oc_float<p> weight = rsv->compute_weight_sum(p_hat);
        return update(u, rsv.sample, weight, rsv.M);
    }
    void truncation(oc_uint<p> limit) noexcept {
        M = ocarina::min(limit, M);
    }
    void process_occluded(oc_bool<p> occluded) noexcept {
        W = ocarina::select(occluded, 0.f, W);
        weight_sum = ocarina::select(occluded, 0.f, weight_sum);
    }
    void update_W(oc_float<p> p_hat) noexcept {
        oc_float<p> denominator = p_hat * M;
        W = ocarina::select(denominator == 0.f, 0.f, weight_sum / denominator);
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