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

    [[nodiscard]] virtual Float3 Li(const LightSampleContext &lsc) const noexcept = 0;
//    [[nodiscard]] virtual Float PDF_Li(const LightSampleContext &lsc, const LightEvalContext &lec) const noexcept = 0;
//    [[nodiscard]] virtual Evaluation evaluate(const LightSampleContext &lsc, const LightEvalContext &lec) const noexcept = 0;
//    [[nodiscard]] virtual LightSample sample_Li(const LightSampleContext &lsc, const Float2 &u) const noexcept = 0;
};
}// namespace vision