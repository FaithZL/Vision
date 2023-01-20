//
// Created by Zero on 29/10/2022.
//

#pragma once

#include "dsl/common.h"

namespace vision {
using namespace ocarina;

struct BxDFFlag {
    static constexpr uchar Unset = 1;
    static constexpr uchar Reflection = 1 << 1;
    static constexpr uchar Transmission = 1 << 2;
    static constexpr uchar Diffuse = 1 << 3;
    static constexpr uchar Glossy = 1 << 4;
    static constexpr uchar Specular = 1 << 5;
    static constexpr uchar NearSpec = 1 << 6;
    // Composite _BxDFFlags_ definitions
    static constexpr uchar DiffRefl = Diffuse | Reflection;
    static constexpr uchar DiffTrans = Diffuse | Transmission;
    static constexpr uchar GlossyRefl = Glossy | Reflection;
    static constexpr uchar GlossyTrans = Glossy | Transmission;
    static constexpr uchar SpecRefl = Specular | Reflection;
    static constexpr uchar SpecTrans = Specular | Transmission;
    static constexpr uchar All = Diffuse | Glossy | Specular | Reflection | Transmission | NearSpec;
};

struct RayState {
public:
    OCRay ray;
    Float ior;
    Uchar medium{InvalidUI8};

public:
    [[nodiscard]] Bool in_medium() const noexcept { return medium != InvalidUI8; }
    [[nodiscard]] Float3 origin() const noexcept { return ray->origin(); }
    [[nodiscard]] Float3 direction() const noexcept { return ray->direction(); }
    [[nodiscard]] Float t_max() const noexcept { return ray->t_max(); }
    [[nodiscard]] Float t_min() const noexcept { return ray->t_min(); }
};

struct RaySample {
    RayState ray_state;
    Float weight{1.f};
};

using VSColor = Float3;

struct ScatterEval {
    VSColor f{make_float3(0.f)};
    Float pdf{0.f};
    [[nodiscard]] Float3 value() const noexcept { return f / pdf; }
    [[nodiscard]] Bool valid() const noexcept { return pdf > 0.f; }
};

struct LightEval {
    VSColor L{make_float3(0.f)};
    Float pdf{0.f};
    [[nodiscard]] Float3 value() const noexcept { return L / pdf; }
    [[nodiscard]] Bool valid() const noexcept { return pdf > 0.f; }
};

struct SampledDirection {
    Float3 wi;
    Bool valid;
};

struct ScatterSample {
    ScatterEval eval;
    Float3 wi{make_float3(0.f)};
    [[nodiscard]] Bool valid() const noexcept {
        return eval.valid();
    }
    virtual ~ScatterSample() = default;
};

struct PhaseSample : public ScatterSample {};

struct BSDFSample : public ScatterSample {
    Uchar flags{BxDFFlag::Unset};
    Float eta{1.f};
};

struct LightSample {
    LightEval eval;
    Float3 p_light{make_float3(0.f)};
    [[nodiscard]] Bool valid() const noexcept {
        return eval.valid();
    }
};

}// namespace vision