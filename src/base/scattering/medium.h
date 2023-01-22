//
// Created by Zero on 29/11/2022.
//

#pragma once

#include "dsl/common.h"
#include "math/optics.h"
#include "base/node.h"
#include "interaction.h"

namespace vision {
using namespace ocarina;

class Sampler;
class SampledWavelengths;

class Medium : public Node {
protected:
    uint _index{};

public:
    using Desc = MediumDesc;

public:
    explicit Medium(const MediumDesc &desc) : Node(desc), _index(desc.index) {}
    ~Medium() override = default;
    virtual VSColor Tr(const OCRay &ray, const SampledWavelengths &swl, Sampler *sampler) const noexcept = 0;
    virtual VSColor sample(const OCRay &ray, Interaction &it,
                           const SampledWavelengths &swl, Sampler *sampler) const noexcept = 0;
};


}// namespace vision