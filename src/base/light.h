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
    DeltaPosition,
    DeltaDirection,
    Area,
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
    [[nodiscard]] virtual LightEval evaluate(const LightSampleContext &p_ref, const LightEvalContext &p_light) const noexcept {
        return {Li(p_ref, p_light), PDF_Li(p_ref, p_light)};
    }
};
}// namespace vision