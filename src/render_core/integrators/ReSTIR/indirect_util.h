//
// Created by Zero on 2023/11/7.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/dsl.h"
#include "base/sampler.h"

namespace vision::ReSTIRIndirect {
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
};
}// namespace vision::ReSTIRIndirect

// clang-format off
OC_STRUCT(vision::ReSTIRIndirect::SurfacePoint, pos, ng) {
    void set_position(Float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
    [[nodiscard]] auto position() const noexcept { return make_float3(pos[0], pos[1], pos[2]); }
    void set_normal(Float3 n) noexcept {
        ng[0] = n[0];
        ng[1] = n[1];
        ng[2] = n[2];
    }
    [[nodiscard]] auto normal() const noexcept { return make_float3(ng[0], ng[1], ng[2]); }
};
// clang-format on

namespace vision::ReSTIRIndirect {
struct RSVSample {
    SurfacePoint sample_point{};
    SurfacePoint visible_point{};
    array<float, 3> u{};
    array<float, 3> Lo{};
};
}// namespace vision::ReSTIRIndirect
OC_STRUCT(vision::ReSTIRIndirect::RSVSample, sample_point, visible_point, u, Lo){
    static constexpr EPort p = D;
    [[nodiscard]] Bool valid() const noexcept {
        return Lo[0] != 0 || Lo[1] != 0 || Lo[2] != 0;
    }
};

namespace vision::ReSTIRIndirect {
using IIRSVSample = Var<RSVSample>;
}

namespace vision::ReSTIRIndirect {
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
}// namespace vision::ReSTIRIndirect

// clang-format off
OC_STRUCT(vision::ReSTIRIndirect::Reservoir, weight_sum, C, W, sample) {
    static constexpr EPort p = D;
    [[nodiscard]] Bool valid() const noexcept {
        return sample->valid();
    }
};
// clang-format on

namespace vision::ReSTIRIndirect {
using IIReservoir = ocarina::Var<Reservoir>;
}