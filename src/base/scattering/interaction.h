//
// Created by Zero on 03/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "math/geometry.h"
#include "base/sample.h"

namespace vision {

using namespace ocarina;

template<typename T>
requires is_vector3_expr_v<T>
struct UVN : Frame<T> {
public:
    using vec_ty = T;

public:
    void set_frame(Frame<T> frame) {
        this->x = frame.x;
        this->y = frame.y;
        this->z = frame.z;
    }
    [[nodiscard]] vec_ty dp_du() const noexcept { return this->x; }
    [[nodiscard]] vec_ty dp_dv() const noexcept { return this->y; }
    [[nodiscard]] vec_ty normal() const noexcept { return this->z; }
    [[nodiscard]] boolean_t<T> valid() const noexcept { return nonzero(normal()); }
};

struct MediumInterface {
public:
    Uchar inside{InvalidUI8};
    Uchar outside{InvalidUI8};

public:
    MediumInterface() = default;
    MediumInterface(Uchar in, Uchar out) : inside(in), outside(out) {}
    explicit MediumInterface(Uchar medium_id) : inside(medium_id), outside(medium_id) {}
    [[nodiscard]] Bool is_transition() const noexcept { return inside != outside; }
    [[nodiscard]] Bool has_inside() const noexcept { return inside != InvalidUI8; }
    [[nodiscard]] Bool has_outside() const noexcept { return outside != InvalidUI8; }
};

template<EPort p = D>
[[nodiscard]] Float phase_HG(Float cos_theta, Float g) {
    Float denom = 1 + sqr(g) + 2 * g * cos_theta;
    return Inv4Pi * (1 - sqr(g)) / (denom * sqrt(denom));
}

class Sampler;

class PhaseFunction {
protected:
    const SampledWavelengths *_swl{};

public:
    virtual void init(Float g, const SampledWavelengths &swl) noexcept = 0;
    [[nodiscard]] virtual Bool valid() const noexcept = 0;

    [[nodiscard]] virtual ScatterEval evaluate(Float3 wo, Float3 wi) const noexcept {
        Float val = f(wo, wi);
        return {{_swl->dimension(), val}, val};
    }
    [[nodiscard]] virtual PhaseSample sample(Float3 wo, Sampler *sampler) const noexcept = 0;
    [[nodiscard]] virtual Float f(Float3 wo, Float3 wi) const noexcept = 0;
};

class HenyeyGreenstein : public PhaseFunction {
private:
    static constexpr float InvalidG = 10;
    Float _g{InvalidG};

public:
    HenyeyGreenstein() = default;
    void init(Float g, const SampledWavelengths &swl) noexcept override {
        _g = g;
        _swl = &swl;
    }
    [[nodiscard]] Float f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] PhaseSample sample(Float3 wo, Sampler *sampler) const noexcept override;
    [[nodiscard]] Bool valid() const noexcept override { return InvalidG != _g; }
};

struct Interaction {
public:
    Float3 pos;
    Float3 wo;
    Float3 time;
    UVN<Float3> g_uvn;

    Float2 uv;
    UVN<Float3> s_uvn;
    Float prim_area{0.f};
    Uint prim_id{InvalidUI32};
    Uint light_id{InvalidUI32};
    Uint mat_id{InvalidUI32};

    // todo optimize volpt and pt
    MediumInterface mi;
    HenyeyGreenstein phase;

public:
    Interaction();
    Interaction(Float3 pos, Float3 wo);
    void init_phase(Float g, const SampledWavelengths &swl);
    [[nodiscard]] Bool has_phase();
    void set_medium(const Uchar &inside, const Uchar &outside);
    [[nodiscard]] Bool has_emission() const noexcept { return light_id != InvalidUI32; }
    [[nodiscard]] Bool has_material() const noexcept { return mat_id != InvalidUI32; }
    [[nodiscard]] Bool valid() const noexcept { return prim_id != InvalidUI32; }
    [[nodiscard]] Bool on_surface() const noexcept { return g_uvn.valid(); }
    [[nodiscard]] OCRay spawn_ray(const Float3 &dir) const noexcept {
        return vision::spawn_ray(pos, g_uvn.normal(), dir);
    }
    [[nodiscard]] RayState spawn_ray_state(const Float3 &dir) const noexcept;
    [[nodiscard]] RayState spawn_ray_state_to(const Float3 &p) const noexcept;
    [[nodiscard]] OCRay spawn_ray_to(const Float3 &p) const noexcept {
        return vision::spawn_ray_to(pos, g_uvn.normal(), p);
    }
};

struct SpacePoint {
    Float3 pos;
    Float3 ng;
    SpacePoint() = default;
    explicit SpacePoint(const Float3 &p) : pos(p) {}
    SpacePoint(const Float3 &p, const Float3 &n)
        : pos(p), ng(n) {}
    explicit SpacePoint(const Interaction &it)
        : pos(it.pos), ng(it.g_uvn.normal()) {}

    [[nodiscard]] Float3 robust_pos(const Float3 &dir) const noexcept {
        Float factor = select(dot(ng, dir) > 0, 1.f, -1.f);
        return offset_ray_origin(pos, ng * factor);
    }

    [[nodiscard]] OCRay spawn_ray(const Float3 &dir) const noexcept {
        return vision::spawn_ray(pos, ng, dir);
    }
    [[nodiscard]] OCRay spawn_ray_to(const Float3 &p) const noexcept{
        return vision::spawn_ray_to(pos, ng, p);
    }
    [[nodiscard]] OCRay spawn_ray_to(const SpacePoint &lsc) const noexcept{
        return vision::spawn_ray_to(pos, ng, lsc.pos, lsc.ng);
    }
};

struct GeometrySurfacePoint : public SpacePoint {
    Float2 uv{};
    GeometrySurfacePoint() = default;
    explicit GeometrySurfacePoint(const Float3 &p) : SpacePoint(p) {}
    explicit GeometrySurfacePoint(const Interaction &it, Float2 uv)
        : SpacePoint(it), uv(uv) {}
    GeometrySurfacePoint(Float3 p, Float3 ng, Float2 uv)
        : SpacePoint{p, ng}, uv(uv) {}
};

/**
 * A point on light
 * used to eval light PDF or lighting to LightSampleContext
 */
struct LightEvalContext : public GeometrySurfacePoint {
    Float PDF_pos{};
    LightEvalContext() = default;
    explicit LightEvalContext(const Float3 &p) : GeometrySurfacePoint(p) {}
    LightEvalContext(const GeometrySurfacePoint &gsp, Float PDF_pos)
        : GeometrySurfacePoint(gsp), PDF_pos(PDF_pos) {}
    LightEvalContext(Float3 p, Float3 ng, Float2 uv, Float PDF_pos)
        : GeometrySurfacePoint{p, ng, uv}, PDF_pos(PDF_pos) {}
    LightEvalContext(const Interaction &si)
        : GeometrySurfacePoint{si, si.uv}, PDF_pos(1.f / si.prim_area) {}
};

struct LightSampleContext : public SpacePoint {
    Float3 ns;
    LightSampleContext() = default;
    LightSampleContext(const Interaction &it)
        : SpacePoint(it), ns(it.s_uvn.normal()) {}
    LightSampleContext(Float3 p, Float3 ng, Float3 ns)
        : SpacePoint{p, ng}, ns(ns) {}
};

struct TextureEvalContext {
    Float3 pos;
    Float2 uv;
    TextureEvalContext() = default;
    TextureEvalContext(const Interaction &si)
        : pos(si.pos), uv(si.uv) {}
};

struct MaterialEvalContext : public TextureEvalContext {
    Float3 wo;
    Float3 ng, ns;
    Float3 dp_dus;
    MaterialEvalContext() = default;
    MaterialEvalContext(const Interaction &si)
        : TextureEvalContext(si),
          wo(si.wo),
          ng(si.g_uvn.normal()),
          ns(si.s_uvn.normal()),
          dp_dus(si.s_uvn.dp_du()) {}
};

}// namespace vision