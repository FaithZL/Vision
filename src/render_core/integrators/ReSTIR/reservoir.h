//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"
#include "dsl/dsl.h"
#include "base/sampler.h"
#include "base/illumination/light.h"
#include "common.h"

namespace vision {
struct DISample {
    uint light_index{InvalidUI32};
    uint prim_id{};
    float2 bary{};
    uint age{};
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
}// namespace vision

// clang-format off
OC_STRUCT(vision,DISample, light_index,prim_id, bary, age, p_hat, pos) {
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
using namespace ocarina;

struct DIReservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_float<p> C{};
    oc_float<p> W{};
    DISample sample{};

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

}// namespace vision

// clang-format off
OC_STRUCT(vision,DIReservoir, weight_sum, C, W, sample) {
    static constexpr EPort p = D;
    [[nodiscard]] Bool valid() const noexcept {
        return sample->valid();
    }
    Bool update(oc_float<p> u, vision::DISampleVar v, oc_float<p> weight, oc_float<p> new_C = 1.f) noexcept {
        weight_sum += weight;
        C += new_C;
        Bool ret = u * weight_sum < weight;
        sample = ocarina::select(ret, v, sample);
        return ret;
    }
    void truncation(oc_float<p> limit) noexcept {
        C = ocarina::min(limit, C);
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
// clang-format on


namespace vision {
struct SurfacePoint {
    array<float, 3> pos{};
    array<float, 3> ng{};
    void set_position(float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
    [[nodiscard]] auto position() const noexcept { return make_float3(pos[0], pos[1], pos[2]); }
    void set_normal(float3 n) noexcept {
        ng[0] = n[0];
        ng[1] = n[1];
        ng[2] = n[2];
    }
    [[nodiscard]] auto normal() const noexcept { return make_float3(ng[0], ng[1], ng[2]); }
    [[nodiscard]] auto valid() const noexcept { return ocarina::any(normal() != 0.f); }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision, SurfacePoint, pos, ng) {
    void set_position(Float3 p) noexcept {
        pos.set(p);
    }
    [[nodiscard]] auto position() const noexcept { return pos.as_vec(); }
    void set_normal(Float3 n) noexcept {
        ng.set(n);
    }
    [[nodiscard]] auto normal() const noexcept { return ng.as_vec(); }
    void set(const vision::Interaction &it) noexcept {
        set_position(it.pos);
        set_normal(it.ng);
    }
    [[nodiscard]] auto valid() const noexcept { return ocarina::any(normal() != 0.f);}
};
// clang-format on

namespace vision {
struct GISample {
    SurfacePoint sp{};
    array<float, 3> Lo{};
    float age{};
};
}// namespace vision

OC_STRUCT(vision, GISample, sp, Lo, age) {
    static constexpr EPort p = D;
    [[nodiscard]] Float p_hat(const Float3 &bsdf) const noexcept {
        return ocarina::luminance(Lo.as_vec() * bsdf);
    }
};

namespace vision {
struct GIReservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_float<p> C{};
    oc_float<p> W{};
    GISample sample{};

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
}// namespace vision

// clang-format off
OC_STRUCT(vision,GIReservoir, weight_sum, C, W, sample) {
    static constexpr EPort p = D;
    Bool update(oc_float<p> u, vision::GISampleVar v, oc_float<p> weight, oc_float<p> new_C = 1.f) noexcept {
        weight_sum += weight;
        C += new_C;
        Bool ret = u * weight_sum < weight || weight_sum == 0;
        sample = ocarina::select(ret, v, sample);
        return ret;
    }
    void truncation(oc_float<p> limit) noexcept {
        oc_float<p> factor = limit / C;
        C = ocarina::min(limit, C);
        weight_sum = ocarina::select(factor < 1.f, weight_sum * factor, weight_sum);
    }
    void process_occluded(oc_bool<p> occluded) noexcept {
        W = ocarina::select(occluded, 0.f, W);
        weight_sum = ocarina::select(occluded, 0.f, weight_sum);
    }
    [[nodiscard]] oc_float<p> cal_W(const oc_float<p> &p_hat) const noexcept {
        return ocarina::select(p_hat == 0.f, 0.f, weight_sum / (p_hat * C));
    }
    void update_W(const oc_float<p> &p_hat) noexcept {
        W = cal_W(p_hat);
    }
};
// clang-format on