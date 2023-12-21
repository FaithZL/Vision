//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class BlackBodyBxDFSet : public BxDFSet {
private:
    const SampledWavelengths *_swl{nullptr};

public:
    explicit BlackBodyBxDFSet(const SampledWavelengths &swl) : _swl(&swl) {}
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        ScatterEval ret{_swl->dimension()};
        ret.f = {_swl->dimension(), 0.f};
        ret.pdf = 1.f;
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        BSDFSample ret{_swl->dimension()};
        return ret;
    }
    [[nodiscard]] SampledSpectrum albedo() const noexcept override {
        return {_swl->dimension(), 0.f};
    }
    VS_MAKE_BxDFSet_ASSIGNMENT(BlackBodyBxDFSet)
};

class BlackBodyMaterial : public Material {
protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<BlackBodyBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        return make_unique<BlackBodyBxDFSet>(swl);
    }
    explicit BlackBodyMaterial(const MaterialDesc &desc)
        : Material(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BlackBodyMaterial)