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
template<typename T, typename Desc>
class TObject;

using TSampler = TObject<Sampler, SamplerDesc>;

class SampledWavelengths;
class ShapeInstance;
class Medium : public Node, public Encodable {
protected:
    uint index_{InvalidUI32};
    float scale_{};

public:
    using Desc = MediumDesc;

public:
    Medium() = default;
    explicit Medium(const MediumDesc &desc)
        : Node(desc),
          scale_(desc.scale["value"].as_float()) {}
    ~Medium() override = default;
    void set_index(uint index) noexcept { index_ = index; }
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    OC_MAKE_MEMBER_GETTER(index,)
    template<typename T>
    void add_reference(T shape_instance) noexcept {}
    virtual SampledSpectrum Tr(const RayVar &ray, const SampledWavelengths &swl,
                               TSampler &sampler) const noexcept = 0;
    virtual SampledSpectrum sample(const RayVar &ray, Interaction &it,
                                   const SampledWavelengths &swl, TSampler &sampler) const noexcept = 0;
};

}// namespace vision