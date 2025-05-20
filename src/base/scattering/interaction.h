//
// Created by Zero on 03/10/2022.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "math/geometry.h"
#include "base/sample.h"
#include "math/optics.h"

namespace vision {
using namespace ocarina;
struct SurfaceData {
public:
    static constexpr uint Miss = BxDFFlag::Unset;
    static constexpr uint NearSpec = BxDFFlag::NearSpec;
    static constexpr uint Glossy = BxDFFlag::Glossy;
    static constexpr uint Diffuse = BxDFFlag::Diffuse;

public:
    TriangleHit hit{};
    float4 normal_depth{};
    float3 pos{};
    uint mat_id{};
    uint is_replaced{false};
    uint is_split{false};
    uint flag{Miss};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision, SurfaceData, hit, normal_depth, pos, mat_id, is_replaced, is_split, flag) {
    void set_normal(const Float3 &n) {
        normal_depth = make_float4(n, normal_depth.w);
    }
    [[nodiscard]] Float3 normal() const noexcept { return normal_depth.xyz();}
    void set_depth(const Float &t) { normal_depth.w = t; }
    void set_position(const Float3 &p) noexcept { pos = p; }
    [[nodiscard]] Float3 position() const noexcept { return pos; }
    [[nodiscard]] Bool near_specular() const noexcept { return flag == vision::SurfaceData::NearSpec; }
    [[nodiscard]] Float depth() const noexcept { return normal_depth.w; }
};
// clang-format on

namespace vision {
struct SurfaceExtend {
    float3 throughput{make_float3(1.f)};
    float3 view_pos{};
    float t_max{};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision, SurfaceExtend, throughput, view_pos, t_max) {};
// clang-format om

namespace vision {
using namespace ocarina;
struct HitBSDF {
public:
    array_float3 wi{};
    array_float3 bsdf{};
    float pdf{-1};
};
}// namespace vision

// clang-format off
OC_STRUCT(vision,HitBSDF, wi, bsdf, pdf) {
    [[nodiscard]] Float3 throughput() const noexcept {
        return bsdf.as_vec3() / pdf;
    }
    [[nodiscard]] Float3 safe_throughput() const noexcept {
        return ocarina::zero_if_nan_inf(throughput());
    }
    [[nodiscard]] Bool valid() const noexcept {
        return pdf > 0.f;
    }
};
// clang-format on

namespace vision {

using namespace ocarina;
template<typename T>
requires is_vector3_expr_v<T>
struct PartialDerivative : Frame<T, false> {
public:
    using vec_ty = T;

public:
    vec_ty dn_du;
    vec_ty dn_dv;

public:
    void set_frame(Frame<T> frame) {
        this->x = frame.x;
        this->y = frame.y;
        this->z = frame.z;
    }
    void update(const vec_ty &norm) noexcept {
        this->z = norm;
        this->x = normalize(cross(norm, this->y)) * length(this->x);
        this->y = normalize(cross(norm, this->x)) * length(this->y);
    }
    [[nodiscard]] vec_ty dp_du() const noexcept { return this->x; }
    [[nodiscard]] vec_ty dp_dv() const noexcept { return this->y; }
    [[nodiscard]] vec_ty normal() const noexcept { return this->z; }
    [[nodiscard]] boolean_t<T> valid() const noexcept { return nonzero(normal()); }
};

struct MediumInterface {
public:
    Uint inside{InvalidUI32};
    Uint outside{InvalidUI32};

public:
    MediumInterface() = default;
    MediumInterface(Uint in, Uint out) : inside(in), outside(out) {}
    explicit MediumInterface(Uint medium_id) : inside(medium_id), outside(medium_id) {}
    [[nodiscard]] Bool is_transition() const noexcept { return inside != outside; }
    [[nodiscard]] Bool has_inside() const noexcept { return inside != InvalidUI32; }
    [[nodiscard]] Bool has_outside() const noexcept { return outside != InvalidUI32; }
};

template<EPort p = D>
[[nodiscard]] Float phase_HG(Float cos_theta, Float g) {
    Float denom = 1 + sqr(g) + 2 * g * cos_theta;
    return Inv4Pi * (1 - sqr(g)) / (denom * sqrt(denom));
}

class Sampler;

class PhaseFunction {
protected:
    const SampledWavelengths *swl_{};

public:
    virtual void init(Float g, const SampledWavelengths &swl) noexcept = 0;
    [[nodiscard]] virtual Bool valid() const noexcept = 0;

    [[nodiscard]] virtual ScatterEval evaluate(const Float3 &wo, const Float3 &wi, MaterialEvalMode mode,
                                               const Uint &, TransportMode) const noexcept {
        Float val = f(wo, wi);
        return {{swl_->dimension(), val}, val, 0};
    }
    [[nodiscard]] virtual PhaseSample sample(const Float3 &wo, TSampler &sampler) const noexcept = 0;
    [[nodiscard]] virtual Float f(const Float3 &wo, const Float3 &wi) const noexcept = 0;
};

class HenyeyGreenstein : public PhaseFunction {
private:
    static constexpr float InvalidG = 10;
    Float g_{InvalidG};

public:
    HenyeyGreenstein() = default;
    void init(Float g, const SampledWavelengths &swl) noexcept override {
        g_ = g;
        swl_ = &swl;
    }
    [[nodiscard]] Float f(const Float3 &wo, const Float3 &wi) const noexcept override;
    [[nodiscard]] PhaseSample sample(const Float3 &wo, TSampler &sampler) const noexcept override;
    [[nodiscard]] Bool valid() const noexcept override { return InvalidG != g_; }
};

template<typename Pos, typename Normal, typename W>
inline auto offset_ray_origin(Pos &&p_in, Normal n_in, W w) noexcept {
    n_in = ocarina::select(ocarina::dot(w, n_in) > 0, n_in, -n_in);
    return offset_ray_origin(OC_FORWARD(p_in), n_in);
}

struct Interaction {
private:
    static float s_ray_offset_factor;

public:
    [[nodiscard]] static float ray_offset_factor() noexcept;
    static void set_ray_offset_factor(float value) noexcept;
    template<typename Pos, typename Normal>
    static auto custom_offset_ray_origin(Pos &&p_in, Normal &&n_in) noexcept {
        float factor = ray_offset_factor();
        return offset_ray_origin(OC_FORWARD(p_in), OC_FORWARD(n_in) * factor);
    }

    template<typename Pos, typename Normal, typename W>
    static auto custom_offset_ray_origin(Pos &&p_in, Normal &&n_in, W w) noexcept {
        float factor = ray_offset_factor();
        return offset_ray_origin(OC_FORWARD(p_in), OC_FORWARD(n_in) * factor, w);
    }

public:
    Float3 pos;
    Float3 wo;
    Float3 time;
    Float3 ng;
    Float3 ng_local;

    Float2 uv;
    Float2 lightmap_uv;
    PartialDerivative<Float3> shading;
    Float prim_area{0.f};
    Uint prim_id{InvalidUI32};
    Uint lightmap_id{InvalidUI32};
    Float du_dx;
    Float du_dy;
    Float dv_dx;
    Float dv_dy;

private:
    Uint mat_id_{InvalidUI32};
    Uint light_id_{InvalidUI32};

private:
    // todo optimize volpt and pt
    optional<MediumInterface> mi_{};
    optional<HenyeyGreenstein> phase_{};

public:
    explicit Interaction(bool has_medium);
    Interaction(Float3 pos, const Float3 &wo, bool has_medium);
    void init_volumetric_param(bool has_medium) noexcept;
    void init_phase(Float g, const SampledWavelengths &swl);
    [[nodiscard]] Bool has_phase();
    [[nodiscard]] float_array correct_eta(const float_array &ior) const noexcept;
    [[nodiscard]] Float correct_eta(const Float &ior) const noexcept;
    void update_wo(const Float3 &view_pos) noexcept { wo = normalize(view_pos - pos); }
    void set_medium(const Uint &inside, const Uint &outside);
    void set_material(const Uint &mat) noexcept { mat_id_ = mat; }
    void set_light(const Uint &light) noexcept { light_id_ = light; }
    [[nodiscard]] Float3 local_wo() const noexcept;
    [[nodiscard]] HenyeyGreenstein phase() const noexcept { return *phase_; }
    [[nodiscard]] MediumInterface mi() const noexcept { return *mi_; }
    [[nodiscard]] Bool has_emission() const noexcept { return light_id_ != InvalidUI32; }
    [[nodiscard]] Bool has_material() const noexcept { return mat_id_ != InvalidUI32; }
    [[nodiscard]] Bool has_lightmap() const noexcept { return lightmap_id != InvalidUI32; }
    [[nodiscard]] Uint material_inst_id() const noexcept;
    [[nodiscard]] Uint material_type_id() const noexcept;
    [[nodiscard]] Uint material_id() const noexcept { return mat_id_; }
    [[nodiscard]] Uint light_inst_id() const noexcept;
    [[nodiscard]] Uint light_type_id() const noexcept;
    [[nodiscard]] Uint light_id() const noexcept { return light_id_; }
    [[nodiscard]] Bool valid() const noexcept { return prim_id != InvalidUI32; }
    [[nodiscard]] RayVar spawn_ray(const Float3 &dir) const noexcept;
    [[nodiscard]] RayVar spawn_ray(const Float3 &dir, const Float &t) const noexcept;
    [[nodiscard]] RayState spawn_ray_state(const Float3 &dir) const noexcept;
    [[nodiscard]] RayState spawn_ray_state_to(const Float3 &p) const noexcept;
    [[nodiscard]] RayVar spawn_ray_to(const Float3 &p) const noexcept;
    [[nodiscard]] Float3 robust_position() const noexcept;
    [[nodiscard]] Float3 robust_position(const Float3 &w) const noexcept;
};

struct HitContext {
public:
    mutable TriangleHitVar *hit{};
    mutable Interaction *it{};

public:
    HitContext() = default;
    HitContext(Interaction &it)
        : it(&it) {}
    HitContext(TriangleHitVar &hit)
        : hit(&hit) {}
    HitContext(TriangleHitVar &hit, Interaction &it) {
        this->hit = &hit;
        this->it = &it;
    }
};

template<typename T>
[[nodiscard]] ray_t<T> spawn_ray(T pos, T normal, T dir) {
    normal *= select(dot(normal, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(pos, normal);
    return make_ray(org, dir);
}

template<typename T, typename U>
[[nodiscard]] ray_t<T> spawn_ray(T pos, T normal, T dir, U t_max) {
    normal *= select(dot(normal, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(pos, normal);
    return make_ray(org, dir, t_max);
}

template<typename T>
[[nodiscard]] ray_t<T> spawn_ray_to(T p_start, T n_start, T p_target) {
    T dir = p_target - p_start;
    n_start *= select(dot(n_start, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(p_start, n_start);
    return make_ray(org, dir, 1 - ShadowEpsilon);
}

template<typename T>
[[nodiscard]] ray_t<T> spawn_ray_to(T p_start, T n_start, T p_target, T n_target) {
    T dir = p_target - p_start;
    n_target *= select(dot(n_target, -dir) > 0, 1.f, -1.f);
    p_target = offset_ray_origin(p_target, n_target);
    n_start *= select(dot(n_start, dir) > 0, 1.f, -1.f);
    T org = offset_ray_origin(p_start, n_start);
    return make_ray(org, dir, 1 - ShadowEpsilon);
}

struct SpacePoint {
    Float3 pos;
    Float3 ng;
    SpacePoint() = default;
    explicit SpacePoint(Float3 p) : pos(std::move(p)) {}
    SpacePoint(Float3 p, Float3 n)
        : pos(std::move(p)), ng(std::move(n)) {}
    explicit SpacePoint(const Interaction &it)
        : pos(it.pos), ng(it.ng) {}

    [[nodiscard]] Float3 robust_pos(const Float3 &dir) const noexcept {
        Float factor = select(dot(ng, dir) > 0, 1.f, -1.f);
        return Interaction::custom_offset_ray_origin(pos, ng * factor);
    }

    [[nodiscard]] RayVar spawn_ray(const Float3 &dir) const noexcept {
        return vision::spawn_ray(pos, ng, dir);
    }
    [[nodiscard]] RayVar spawn_ray_to(const Float3 &p) const noexcept {
        return vision::spawn_ray_to(pos, ng, p);
    }
    [[nodiscard]] RayVar spawn_ray_to(const SpacePoint &lsc) const noexcept {
        return vision::spawn_ray_to(pos, ng, lsc.pos, lsc.ng);
    }
};

struct GeometrySurfacePoint : public SpacePoint {
    Float2 uv{};
    GeometrySurfacePoint() = default;
    using SpacePoint::SpacePoint;
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
    using GeometrySurfacePoint::GeometrySurfacePoint;
    LightEvalContext() = default;
    LightEvalContext(const GeometrySurfacePoint &gsp, Float PDF_pos)
        : GeometrySurfacePoint(gsp), PDF_pos(std::move(PDF_pos)) {}
    LightEvalContext(Float3 p, Float3 ng, Float2 uv, Float PDF_pos)
        : GeometrySurfacePoint{std::move(p), std::move(ng), std::move(uv)},
          PDF_pos(std::move(PDF_pos)) {}
    LightEvalContext(const Interaction &it)
        : GeometrySurfacePoint{it, it.uv}, PDF_pos(1.f / it.prim_area) {}
};

struct LightSampleContext : public SpacePoint {
    Float3 ns;
    LightSampleContext() = default;
    LightSampleContext(const Interaction &it)
        : SpacePoint(it), ns(it.shading.normal()) {}
    LightSampleContext(Float3 p, Float3 ng, Float3 ns)
        : SpacePoint{std::move(p), std::move(ng)},
          ns(std::move(ns)) {}
};

enum GeometryTag : uint {
    None = 0,
    Position = 1 << 0,
    Wo = 1 << 1,
    Ng = 1 << 2,
    NgLocal = 1 << 3,
    Ns = 1 << 4,
};

struct AttrEvalOutput {
    GeometryTag tag{None};
    float_array array{1u};
    AttrEvalOutput() = default;
    AttrEvalOutput(const float_array &array) : array(array) {}
    AttrEvalOutput(GeometryTag tag, const float_array &array) : array(array), tag(tag) {}
    [[nodiscard]] const auto *operator->() const noexcept { return &array; }
    [[nodiscard]] auto *operator->() noexcept { return &array; }
};

struct AttrEvalInput {
    Float2 uv;
    optional<Float3> pos;
    optional<Float3> wo;
    optional<Float3> ng;
    optional<Float3> ng_local;
    optional<Float3> ns;
    AttrEvalInput() = default;
    AttrEvalInput(Float3 pos)
        : pos(std::move(pos)) {}
    AttrEvalInput(const Interaction &it)
        : pos(it.pos), uv(it.uv) {}
    AttrEvalInput(Float2 uv)
        : uv(std::move(uv)) {}
    AttrEvalInput(const float_array &f_array)
        : uv{f_array.as_vec2()} {
    }
    AttrEvalInput(const AttrEvalOutput &output)
        : AttrEvalInput(output.array.as_vec2()) {
        from_output(output);
    }
    void for_each_optional(const std::function<void(const optional<Float3> &, uint)> &func) const noexcept {
        const optional<Float3> *head = addressof(pos);
        const optional<Float3> *last = addressof(ns);
        uint i = 0;
        uint tag = 0;
        for (const optional<Float3> *ptr = head; ptr <= last; ++ptr, ++i) {
            func(*ptr, i);
        }
    }
    [[nodiscard]] static uint float_num(GeometryTag tag) noexcept;
    [[nodiscard]] AttrEvalOutput to_output() const noexcept;
    [[nodiscard]] GeometryTag compute_tag() const noexcept;
    void from_output(const AttrEvalOutput &input) noexcept;
};

}// namespace vision