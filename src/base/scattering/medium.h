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
class ShapeGroup;
class Medium : public Node, public Encodable, public std::enable_shared_from_this<Medium> {
protected:
    uint index_{InvalidUI32};
    EncodedData<float> scale_{};
    vector<weak_ptr<ShapeInstance>> inside_ref_instances;
    vector<weak_ptr<ShapeInstance>> outside_ref_instances;

public:
    using Desc = MediumDesc;

public:
    Medium() = default;
    explicit Medium(const MediumDesc &desc)
        : Node(desc),
          scale_(desc.scale["value"].as_float()) {}
    OC_ENCODABLE_FUNC(Encodable, scale_)
    void restore(vision::RuntimeObject *old_obj) noexcept override;
    ~Medium() override = default;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    void add_inside_reference(SP<ShapeInstance> shape_instance) noexcept;
    void add_outside_reference(SP<ShapeInstance> shape_instance) noexcept;
    virtual SampledSpectrum Tr(const RayVar &ray, const SampledWavelengths &swl,
                               TSampler &sampler) const noexcept = 0;
    virtual SampledSpectrum sample(const RayState &rs, Interaction &it, TSampler &sampler,
                                   const SampledWavelengths &swl) const noexcept = 0;
};

}// namespace vision