//
// Created by Zero on 29/10/2022.
//

#pragma once

#include <utility>

#include "dsl/dsl.h"
#include "base/color/spectrum.h"

namespace vision {
enum MaterialEvalMode {
    F = 1 << 0,
    PDF = 1 << 1,
    All = F | PDF
};
}
OC_MAKE_ENUM_BIT_OPS(vision::MaterialEvalMode, |, &, <<, >>)

namespace vision {
using namespace ocarina;

struct BxDFFlag {
    static constexpr uint Unset = 1;
    static constexpr uint Reflection = 1 << 1;
    static constexpr uint Transmission = 1 << 2;
    static constexpr uint Diffuse = 1 << 3;
    static constexpr uint Glossy = 1 << 4;
    static constexpr uint Specular = 1 << 5;
    static constexpr uint NearSpec = 1 << 6;
    // Composite _BxDFFlags_ definitions
    static constexpr uint DiffRefl = Diffuse | Reflection;
    static constexpr uint DiffTrans = Diffuse | Transmission;
    static constexpr uint GlossyRefl = Glossy | Reflection;
    static constexpr uint GlossyTrans = Glossy | Transmission;
    static constexpr uint SpecRefl = Specular | Reflection;
    static constexpr uint SpecTrans = Specular | Transmission;
    static constexpr uint All = Diffuse | Glossy | Specular | Reflection | Transmission | NearSpec;

    static Bool is_reflection(const Uint &f) noexcept { return f & Reflection; }
    static Bool is_transmission(const Uint &f) noexcept { return f & Transmission; }
    static Bool is_diffuse(const Uint &f) noexcept { return f & Diffuse; }
    static Bool is_glossy(const Uint &f) noexcept { return f & Glossy; }
    static Bool is_specular(const Uint &f) noexcept { return f & Specular; }
};

struct RayState {
public:
    RayVar ray;
    Float ior;
    Uint medium{InvalidUI32};

public:
    static RayState create(const RayVar &ray, Float ior = 1.f,
                           Uint medium = InvalidUI32) noexcept {
        return {.ray = ray, .ior = std::move(ior), .medium = std::move(medium)};
    }
    [[nodiscard]] Bool in_medium() const noexcept { return medium != InvalidUI32; }
    [[nodiscard]] Float3 origin() const noexcept { return ray->origin(); }
    [[nodiscard]] Float3 direction() const noexcept { return ray->direction(); }
    [[nodiscard]] Float t_max() const noexcept { return ray->t_max(); }
    [[nodiscard]] Float t_min() const noexcept { return ray->t_min(); }
};

struct RaySample {
    RayState ray_state;
    Float weight{1.f};
};

struct ScatterEval {
public:
    SampledSpectrum f{};
    Uint flags{BxDFFlag::Unset};
    DynamicArray<float> pdfs{1};

public:
    explicit ScatterEval(const SampledWavelengths &swl)
        : ScatterEval(swl.dimension(), swl.scatter_pdf_dim()) {}
    explicit ScatterEval(uint dim, uint pdf_dim) : f(dim), pdfs(pdf_dim, 0.f){};
    ScatterEval(SampledSpectrum f, Float pdf, Uint flags)
        : f(std::move(f)), pdfs(1, std::move(pdf)), flags(std::move(flags)) {}
    [[nodiscard]] SampledSpectrum throughput() const noexcept { return f / pdfs; }
    [[nodiscard]] SampledSpectrum safe_throughput() const noexcept {
        auto ret = throughput();
        ret.sanitize();
        return ret;
    }
    [[nodiscard]] Bool valid() const noexcept {
        return pdfs.all([&](const Float &v) -> Bool {
            return v > 0.f;
        });
    }
    [[nodiscard]] Float &pdf() noexcept { return pdfs[0]; }
    [[nodiscard]] Float pdf() const noexcept { return pdfs[0]; }
    void invalidation() noexcept { pdfs[0] = 0; }
};

struct LightEval {
public:
    SampledSpectrum L{};
    Float pdf{0.f};

public:
    explicit LightEval(uint dim) : L(dim), pdf{0.f} {};
    LightEval(SampledSpectrum L, Float pdf)
        : L(std::move(L)), pdf(std::move(pdf)) {}
    [[nodiscard]] SampledSpectrum value() const noexcept { return L / pdf; }
    [[nodiscard]] Bool valid() const noexcept { return pdf > 0.f; }
    void invalidation() noexcept { pdf = 0; }
};

struct SampledDirection {
    Float3 wi;
    Bool valid{true};
    [[nodiscard]] Float factor() const noexcept { return cast<float>(valid); }
};

struct ScatterSample {
public:
    ScatterEval eval;
    Float3 wi{make_float3(0.f)};

public:
    explicit ScatterSample(const SampledWavelengths &swl)
        : ScatterSample(swl.dimension(), swl.scatter_pdf_dim()) {}
    explicit ScatterSample(uint dim, uint pdf_dim) noexcept
        : eval(dim, pdf_dim), wi(make_float3(0.f)) {}
    [[nodiscard]] Bool valid() const noexcept { return eval.valid(); }
    void invalidation() noexcept { eval.invalidation(); }
    virtual ~ScatterSample() = default;
};

struct PhaseSample : public ScatterSample {
    using ScatterSample::ScatterSample;
    explicit PhaseSample(uint dim) : ScatterSample(dim, 1){};
};

struct BSDFSample : public ScatterSample {
public:
    Float eta{1.f};

public:
    using ScatterSample::ScatterSample;
};

struct LightSample {
public:
    LightEval eval;
    Float3 p_light{make_float3(0.f)};

public:
    explicit LightSample(uint dim) noexcept
        : eval(dim), p_light(make_float3(0.f)) {}
    [[nodiscard]] Bool valid() const noexcept {
        return eval.valid();
    }
};

}// namespace vision