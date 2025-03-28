//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/scattering/interaction.h"
#include "base/node.h"
#include "base/shader_graph/shader_graph.h"
#include "base/sample.h"
#include "base/encoded_object.h"
#include "base/color/spectrum.h"
#include "math/warp.h"

namespace vision {
struct LightBound {
    ocarina::array<float, 3> axis_{};
    float theta_o{};
    float theta_e{};
    [[nodiscard]] auto axis() const noexcept {
        return make_float3(axis_[0], axis_[1], axis_[2]);
    }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision,LightBound, axis_, theta_o, theta_e){
    [[nodiscard]] auto axis() const noexcept {
        return axis_.as_vec3();
    }
};
// clang-format on

namespace vision {
enum class LightEvalMode {
    L = 1 << 0,
    PDF = 1 << 1,
    All = L | PDF
};
}// namespace vision

OC_MAKE_ENUM_BIT_OPS(vision::LightEvalMode, |, &, <<, >>)

namespace vision {
// LightType Definition
enum class LightType {
    Unset = 0,
    Area = 1 << 0,
    DeltaPosition = 1 << 1,
    DeltaDirection = 1 << 2,
    Infinite = 1 << 3
};
}// namespace vision

OC_MAKE_ENUM_BIT_OPS(vision::LightType, |, &, <<, >>)

#undef VS_MAKE_LIGHT_TYPE_OP

namespace vision {

struct LightSurfacePoint {
    Uint light_index{InvalidUI32};
    Uint prim_id;
    Float2 bary;
};

class Light : public Node, public Encodable, public ShaderGraph {
public:
    using Desc = LightDesc;

protected:
    const LightType type_{LightType::Unset};
    EncodedData<float> scale_{1.f};
    EncodedData<uint> switch_{true};
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(strength);
    uint index_{InvalidUI32};

protected:
    [[nodiscard]] float3 average() const noexcept {
        auto a = color_.average();
        return make_float3(a[0], a[1], a[2]) * scale_.hv();
    }

    [[nodiscard]] static bool match_L(LightEvalMode mode) noexcept {
        return static_cast<bool>(mode & LightEvalMode::L);
    }

    [[nodiscard]] static bool match_PDF(LightEvalMode mode) noexcept {
        return static_cast<bool>(mode & LightEvalMode::PDF);
    }

    [[nodiscard]] virtual LightEval _evaluate_point(const LightSampleContext &p_ref,
                                                    const LightEvalContext &p_light,
                                                    const SampledWavelengths &swl,
                                                    LightEvalMode mode) const noexcept {
        LightEval ret{swl.dimension()};
        if (match_L(mode)) {
            ret.L = Li(p_ref, p_light, swl);
        }
        if (match_PDF(mode)) {
            ret.pdf = PDF_point(p_ref, p_light);
        }
        return ret;
    }

public:
    Light(LightType type) noexcept : type_(type) {}
    explicit Light(const LightDesc &desc, LightType light_type);
    VS_HOTFIX_MAKE_RESTORE(Node, scale_, switch_, color_, strength_, index_)
    OC_ENCODABLE_FUNC(Encodable, scale_, color_, strength_, switch_)
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return color_.type_hash();
    }
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    VS_MAKE_GUI_STATUS_FUNC(Node, color_, strength_)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] virtual LightBound bound() const noexcept { return {}; }
    [[nodiscard]] virtual float3 power() const noexcept = 0;
    [[nodiscard]] Float scale() const noexcept { return *scale_ * *switch_; }
    [[nodiscard]] virtual SampledSpectrum Le(const LightSampleContext &p_ref,
                                             const LightEvalContext &p_light,
                                             const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] static Float G(const LightSampleContext &p_ref,
                                 const LightEvalContext &p_light) noexcept {
        Float3 w = p_ref.pos - p_light.pos;
        return dot(normalize(w), p_light.ng) / length_squared(w);
    }
    [[nodiscard]] virtual SampledSpectrum Li(const LightSampleContext &p_ref,
                                             const LightEvalContext &p_light,
                                             const SampledWavelengths &swl) const noexcept {
        return Le(p_ref, p_light, swl);
    }
    [[nodiscard]] virtual Float PMF(const Uint &prim_id) const noexcept { return 0.f; }
    [[nodiscard]] virtual Float PDF_wi(const LightSampleContext &p_ref,
                                       const LightEvalContext &p_light) const noexcept = 0;
    [[nodiscard]] virtual Float PDF_point(const LightSampleContext &p_ref,
                                          const LightEvalContext &p_light) const noexcept {
        return PDF_wi(p_ref, p_light);
    }
    [[nodiscard]] virtual Float PDF_point(const LightSampleContext &p_ref,
                                          const LightEvalContext &p_light,
                                          const Float &pdf_wi) const noexcept {
        Float ret = vision::PDF_point(pdf_wi, p_light.ng, p_ref.pos - p_light.pos);
        return select(ocarina::isinf(ret), 0.f, ret);
    }
    [[nodiscard]] Float PDF_point(const LightSampleContext &p_ref,
                                  LightSurfacePoint lsp,
                                  const Float &pdf_wi) const noexcept {
        return PDF_point(p_ref, compute_light_eval_context(p_ref, lsp), pdf_wi);
    }
    [[nodiscard]] virtual LightSample sample_wi(const LightSampleContext &p_ref, Float2 u,
                                                const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] LightType type() const noexcept { return type_; }
    [[nodiscard]] bool match(LightType t) const noexcept { return static_cast<bool>(t & type_); }
    [[nodiscard]] bool is(LightType t) const noexcept { return t == type_; }
    [[nodiscard]] virtual LightEval evaluate_wi(const LightSampleContext &p_ref,
                                                const LightEvalContext &p_light,
                                                const SampledWavelengths &swl,
                                                LightEvalMode mode) const noexcept {
        LightEval ret{swl.dimension()};
        if (match_L(mode)) {
            ret.L = Le(p_ref, p_light, swl);
        }
        if (match_PDF(mode)) {
            ret.pdf = PDF_wi(p_ref, p_light);
        }
        return ret;
    }

    [[nodiscard]] virtual LightSample evaluate_point(const LightSampleContext &p_ref, LightSurfacePoint lsp,
                                                     const SampledWavelengths &swl, LightEvalMode mode) const noexcept = 0;

    [[nodiscard]] virtual LightEvalContext compute_light_eval_context(const LightSampleContext &p_ref,
                                                                      LightSurfacePoint lsp) const noexcept = 0;

    [[nodiscard]] virtual LightSurfacePoint sample_only(Float2 u) const noexcept {
        LightSurfacePoint ret;
        ret.bary = u;
        ret.prim_id = 0;
        return ret;
    }

    [[nodiscard]] LightEval evaluate_point(const LightSampleContext &p_ref,
                                           const LightEvalContext &p_light,
                                           const Float &pdf_wi,
                                           const SampledWavelengths &swl,
                                           LightEvalMode mode) const noexcept {
        LightEval ret{swl.dimension()};
        if (match_L(mode)) {
            ret.L = Li(p_ref, p_light, swl);
        }
        if (match_PDF(mode)) {
            ret.pdf = PDF_point(p_ref, p_light, pdf_wi);
        }
        return ret;
    }
};

class Mesh;
class ShapeInstance;

class IAreaLight : public Light {
protected:
    EncodedData<uint> inst_idx_{InvalidUI32};

public:
    IAreaLight() : Light(LightType::Area) {}
    explicit IAreaLight(const LightDesc &desc)
        : Light(desc, LightType::Area),
          inst_idx_(desc["inst_id"].as_uint(InvalidUI32)) {}
    OC_ENCODABLE_FUNC(Light, inst_idx_)
    VS_HOTFIX_MAKE_RESTORE(Light, inst_idx_)
    template<typename T>
    void add_reference(T shape_instance) noexcept {}
    [[nodiscard]] ShapeInstance *instance() const noexcept;
};

class IPointLight : public Light {
public:
    IPointLight(): Light(LightType::DeltaPosition) {}
    explicit IPointLight(const LightDesc &desc) : Light(desc, LightType::DeltaPosition) {}
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] Float PDF_wi(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        // using -1 for delta light
        return -1.f;
    }
    [[nodiscard]] virtual Float3 direction(const LightSampleContext &p_ref) const noexcept {
        return normalize(p_ref.pos - position());
    }
    [[nodiscard]] virtual Float3 position() const noexcept = 0;
    [[nodiscard]] virtual float3 &host_position() noexcept = 0;
    [[nodiscard]] LightSample sample_wi(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override;
    [[nodiscard]] LightSample evaluate_point(const LightSampleContext &p_ref, LightSurfacePoint lsp,
                                             const SampledWavelengths &swl, LightEvalMode mode) const noexcept override {
        LightSample ls{swl.dimension()};
        LightEvalContext lec{position()};
        ls.eval = _evaluate_point(p_ref, lec, swl, mode);
        ls.p_light = position();
        return ls;
    }
    [[nodiscard]] LightEvalContext compute_light_eval_context(const LightSampleContext &p_ref,
                                                              LightSurfacePoint lsp) const noexcept override {
        return LightEvalContext{position(), normalize(p_ref.pos - position())};
    }
};

class Environment : public Light {
public:
    using Light::Light;
    [[nodiscard]] virtual Float2 convert_to_bary(const Float3 &world_dir) const noexcept {
        return make_float2(0.f);
    }
};

using TLight = TObject<Light>;
using TEnvironment = TObject<Environment>;

}// namespace vision