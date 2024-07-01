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

class SamplerImpl;
template<typename T, typename Desc>
class TObject;

using Sampler = TObject<SamplerImpl, SamplerDesc>;

class SampledWavelengths;

class Medium : public Node, public Encodable<float> {
protected:
    uint index_{InvalidUI32};
    float scale_{};

public:
    using Desc = MediumDesc;

public:
    explicit Medium(const MediumDesc &desc)
        : Node(desc),
          scale_(desc.scale["value"].as_float()) {}
    ~Medium() override = default;
    void set_index(uint index) noexcept { index_ = index; }
    OC_MAKE_MEMBER_GETTER(index,)
    virtual SampledSpectrum Tr(const RayVar &ray, const SampledWavelengths &swl,
                               Sampler &sampler) const noexcept = 0;
    virtual SampledSpectrum sample(const RayVar &ray, Interaction &it,
                                   const SampledWavelengths &swl, Sampler &sampler) const noexcept = 0;
};

}// namespace vision