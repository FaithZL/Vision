//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/scattering/interaction.h"
#include "base/node.h"
#include "base/shader_graph/shader_node.h"
#include "base/sample.h"
#include "base/color/spectrum.h"

namespace vision {
struct LightBound {
    ocarina::array<float, 3> _axis{};
    float theta_o{};
    float theta_e{};
    [[nodiscard]] auto axis() const noexcept {
        return make_float3(_axis[0], _axis[1], _axis[2]);
    }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::LightBound, _axis, theta_o, theta_e){
    [[nodiscard]] auto axis() const noexcept {
        return make_float3(_axis[0], _axis[1], _axis[2]);
    }
};
// clang-format on

namespace vision {

// LightType Definition
enum class LightType {
    Area,
    DeltaPosition,
    DeltaDirection,
    Infinite
};

class Light : public Node, public Serializable<float> {
public:
    using Desc = LightDesc;

protected:
    const LightType _type{LightType::Area};
    Serial<float> _scale{1.f};
    Slot _color{};
    uint _index{InvalidUI32};

protected:
    [[nodiscard]] float3 average() const noexcept {
        auto a = _color.average();
        return make_float3(a[0], a[1], a[2]) * _scale.hv();
    }

public:
    explicit Light(const LightDesc &desc, LightType light_type);
    OC_SERIALIZABLE_FUNC(Serializable<float>, _scale, *_color.node())
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return _color.type_hash();
    }
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    [[nodiscard]] virtual LightBound bound() const noexcept { return {}; }
    [[nodiscard]] virtual float3 power() const noexcept = 0;
    [[nodiscard]] Float scale() const noexcept { return *_scale; }
    [[nodiscard]] virtual SampledSpectrum Li(const LightSampleContext &p_ref, const LightEvalContext &p_light, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual Float PMF(const Uint &prim_id) const noexcept { return 0.f; }
    [[nodiscard]] virtual Float PDF_Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept = 0;
    [[nodiscard]] virtual LightSample sample_Li(const LightSampleContext &p_ref, Float2 u, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] LightType type() const noexcept { return _type; }
    [[nodiscard]] virtual LightEval evaluate(const LightSampleContext &p_ref, const LightEvalContext &p_light, const SampledWavelengths &swl) const noexcept {
        return {Li(p_ref, p_light, swl), PDF_Li(p_ref, p_light)};
    }
};

class Mesh;
class Instance;

class IAreaLight : public Light {
protected:
    Serial<uint> _inst_idx{InvalidUI32};
    const vision::Mesh *_mesh{};
    const Instance *_instance{};

public:
    explicit IAreaLight(const LightDesc &desc)
        : Light(desc, LightType::Area),
          _inst_idx(desc["inst_id"].as_uint(InvalidUI32)) {}
    OC_SERIALIZABLE_FUNC(Light, _inst_idx)
    void set_mesh(const vision::Mesh *m) noexcept;
    void set_instance(const Instance *inst) noexcept;
    [[nodiscard]] vision::Mesh *mesh() const noexcept;
};

class IPointLight : public Light {
public:
    explicit IPointLight(const LightDesc &desc) : Light(desc, LightType::DeltaPosition) {}
    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        // using -1 for delta position light
        return -1.f;
    }
    [[nodiscard]] virtual Float3 position() const noexcept = 0;
    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override;
};

}// namespace vision