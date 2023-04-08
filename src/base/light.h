//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/scattering/interaction.h"
#include "node.h"
#include "shader_graph/shader_node.h"
#include "sample.h"
#include "base/color/spectrum.h"

namespace vision {

// LightType Definition
enum class LightType {
    Area,
    DeltaPosition,
    DeltaDirection,
    Infinite
};

class Light : public Node, public PolymorphicElement<float> {
public:
    using Desc = LightDesc;

protected:
    const LightType _type{LightType::Area};
    Slot _color{};
    float _scale{1.f};

public:
    explicit Light(const LightDesc &desc, LightType light_type);
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return _color.type_hash();
    }
    [[nodiscard]] uint data_size() const noexcept override { return 0u; }
    [[nodiscard]] void fill_data(ManagedWrapper<float> &datas) const noexcept override {

    }
    [[nodiscard]] virtual SampledSpectrum Li(const LightSampleContext &p_ref, const LightEvalContext &p_light, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual Float PMF(const Uint &prim_id) const noexcept { return 0.f; }
    [[nodiscard]] virtual Float PDF_Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept = 0;
    [[nodiscard]] virtual LightSample sample_Li(const LightSampleContext &p_ref, Float2 u, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] LightType type() const noexcept { return _type; }
    [[nodiscard]] virtual LightEval evaluate(const LightSampleContext &p_ref, const LightEvalContext &p_light, const SampledWavelengths &swl) const noexcept {
        return {Li(p_ref, p_light, swl), PDF_Li(p_ref, p_light)};
    }
};

class IPointLight : public Light {
public:
    explicit IPointLight(const LightDesc &desc) : Light(desc, LightType::DeltaPosition) {}
    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        // using -1 for delta position light
        return -1.f;
    }
    [[nodiscard]] virtual float3 position() const noexcept = 0;
    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override;
};

}// namespace vision