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
    SurfacePoint sample_point;
    SurfacePoint visible_point;
    array<float, 3> u{};
    array<float, 3> Lo;
};
}// namespace vision::ReSTIRIndirect
OC_STRUCT(vision::ReSTIRIndirect::RSVSample, sample_point, visible_point, u, Lo){};

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
};
}// namespace vision::ReSTIRIndirect

namespace vision::ReSTIRIndirect {
using IIReservoir = ocarina::Var<Reservoir>;
}