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
    uint light_index{InvalidUI32};
    uint prim_id{};
    float2 bary{};
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
OC_STRUCT(vision::ReSTIRDirect::RSVSample, light_index,prim_id, bary, p_hat, pos) {
    void init() noexcept {
        light_index = InvalidUI32;
    }
    [[nodiscard]] Bool valid() const noexcept {
        return light_index != InvalidUI32;
    }
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_lsp(const vision::LightSurfacePoint &lsp) noexcept {
        bary = lsp.bary;
        light_index = lsp.light_index;
        prim_id = lsp.prim_id;
    }
    [[nodiscard]] vision::LightSurfacePoint lsp() const noexcept {
        vision::LightSurfacePoint lsp;
        lsp.bary = bary;
        lsp.light_index = light_index;
        lsp.prim_id = prim_id;
        return lsp;
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
    oc_float<p> W{};
    RSVSample sample{};

    template<EPort p_ = D>
    [[nodiscard]] static oc_float<p_> cal_weight(oc_float<p_> mis_weight, oc_float<p_> p_hat,
                                                 oc_float<p_> W) noexcept {
        return mis_weight * p_hat * W;
    }

    template<EPort p_ = D>
    [[nodiscard]] static auto safe_weight(oc_float<p_> mis_weight, oc_float<p_> p_hat,
                                          oc_float<p_> W) noexcept {
        oc_float<p_> ret = cal_weight(mis_weight, p_hat, W);
        ret = ocarina::select(ocarina::isnan(ret), 0.f, ret);
        return ret;
    }
};
}// namespace ReSTIRDirect

}// namespace vision

OC_STRUCT(vision::ReSTIRDirect::Reservoir, weight_sum, M, W, sample) {
    static constexpr EPort p = D;
    void init() noexcept {
        sample->init();
    }
    [[nodiscard]] Bool valid() const noexcept {
        return sample->valid();
    }
    Bool update(oc_float<p> u, vision::DIRSVSample v, oc_float<p> weight, oc_float<p> sample_M = 1.f) noexcept {
        weight_sum += weight;
        M += sample_M;
        Bool ret = u * weight_sum < weight;
        sample = select(ret, v, sample);
        return ret;
    }
    void truncation(oc_float<p> limit) noexcept {
        M = ocarina::min(limit, M);
    }
    void process_occluded(oc_bool<p> occluded) noexcept {
        W = ocarina::select(occluded, 0.f, W);
        weight_sum = ocarina::select(occluded, 0.f, weight_sum);
    }
    [[nodiscard]] oc_float<p> cal_W(const oc_float<p> &p_hat) const noexcept {
        return ocarina::select(p_hat == 0.f, 0.f, weight_sum / p_hat);
    }
    void update_W(const oc_float<p> &p_hat) noexcept {
        W = cal_W(p_hat);
    }
};

namespace vision {
using namespace ReSTIRDirect;
using namespace ocarina;
using DIReservoir = Var<ReSTIRDirect::Reservoir>;
}// namespace vision