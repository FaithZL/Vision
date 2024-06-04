//
// Created by Zero on 2023/11/7.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"
#include "dsl/dsl.h"
#include "base/sampler.h"

namespace vision::indirect {
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
}// namespace vision::indirect

// clang-format off
OC_STRUCT(vision::indirect, SurfacePoint, pos, ng) {
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

namespace vision::indirect {
struct RSVSample {
    SurfacePoint sp{};
    SurfacePoint vp{};
    array<float, 3> u{};
    array<float, 3> Lo{};
    float age{};
};
}// namespace vision::indirect

OC_STRUCT(vision::indirect, RSVSample, sp, vp, u, Lo, age) {
    static constexpr EPort p = D;
    [[nodiscard]] Bool valid() const noexcept {
        return vp->valid();
    }
    [[nodiscard]] Float p_hat(const Float3 &bsdf) const noexcept {
        return ocarina::luminance(Lo.as_vec() * bsdf);
    }
    [[nodiscard]] Float p_hat(const Float &cos_theta) const noexcept {
        return ocarina::luminance(Lo.as_vec() * cos_theta);
    }
};

namespace vision {
using GIRSVSample = Var<indirect::RSVSample>;
}

namespace vision::indirect {
struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_float<p> C{};
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
}// namespace vision::indirect

// clang-format off
OC_STRUCT(vision::indirect,Reservoir, weight_sum, C, W, sample) {
    static constexpr EPort p = D;
    [[nodiscard]] Bool valid() const noexcept {
        return sample->valid();
    }
    Bool update(oc_float<p> u, vision::GIRSVSample v, oc_float<p> weight, oc_float<p> new_C = 1.f) noexcept {
        weight_sum += weight;
        C += new_C;
        Bool ret = u * weight_sum <= weight;
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

namespace vision::indirect {
using GIReservoir = ocarina::Var<Reservoir>;
}