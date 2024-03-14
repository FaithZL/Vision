//
// Created by Zero on 29/11/2022.
//

#pragma once

#include "dsl/dsl.h"
#include "math/optics.h"
#include "base/node.h"
#include "interaction.h"

namespace vision {
using namespace ocarina;

class Sampler;
class SampledWavelengths;

class Medium : public Node, public Serializable<float> {
protected:
    uint _index{InvalidUI32};
    float _scale{};

public:
    using Desc = MediumDesc;

public:
    explicit Medium(const MediumDesc &desc)
        : Node(desc),
          _scale(desc.scale["value"].as_float()) {}
    ~Medium() override = default;
    void set_index(uint index) noexcept { _index = index; }
    OC_MAKE_MEMBER_GETTER(index,)
    virtual SampledSpectrum Tr(const RayVar &ray, const SampledWavelengths &swl,
                               Sampler *sampler) const noexcept = 0;
    virtual SampledSpectrum sample(const RayVar &ray, Interaction &it,
                                   const SampledWavelengths &swl, Sampler *sampler) const noexcept = 0;
};

}// namespace vision