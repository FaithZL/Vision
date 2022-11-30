//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "interaction.h"
#include "node.h"
#include "sample.h"

namespace vision {

// LightType Definition
enum class LightType {
    Area,
    DeltaPosition,
    DeltaDirection,
    Infinite
};

class Light : public Node {
public:
    using Desc = LightDesc;

protected:
    const LightType _type{LightType::Area};

public:
    explicit Light(const LightDesc &desc, LightType light_type)
        : Node(desc), _type(light_type) {}

    [[nodiscard]] virtual Float3 Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept = 0;
    [[nodiscard]] virtual Float PMF(const Uint &prim_id) const noexcept { return 0.f; }
    [[nodiscard]] virtual Float PDF_Li(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept = 0;
    [[nodiscard]] virtual LightSample sample_Li(const LightSampleContext &p_ref, Float2 u) const noexcept = 0;
    [[nodiscard]] LightType type() const noexcept { return _type; }
    [[nodiscard]] virtual LightEval evaluate(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept {
        return {Li(p_ref, p_light), PDF_Li(p_ref, p_light)};
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
    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u) const noexcept override {
        LightSample ret;
        LightEvalContext p_light;
        p_light.pos = position();
        ret.eval = evaluate(p_ref, p_light);
        Float3 wi_un = position() - p_ref.pos;
        ret.p_light = p_light.pos;
        return ret;
    }
};

}// namespace vision