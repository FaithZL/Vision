//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class NullBxDFSet : public BxDFSet {
private:
    const SampledWavelengths &_swl;

public:
    explicit NullBxDFSet(const SampledWavelengths &swl) : _swl(swl) {}
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        ScatterEval ret{_swl.dimension()};
        ret.f = {_swl.dimension(), 0.f};
        ret.pdf = 1.f;
        return ret;
    }

    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        BSDFSample ret{_swl.dimension()};
        return ret;
    }

    [[nodiscard]] SampledSpectrum albedo() const noexcept override {
        return {_swl.dimension(), 0.f};
    }
};

class NullMaterial : public Material {
public:
    explicit NullMaterial(const MaterialDesc &desc)
        : Material(desc) {}

    [[nodiscard]] BSDF compute_BSDF(const Interaction &it,
                                    const SampledWavelengths &swl) const noexcept override {
        return BSDF(it, make_unique<NullBxDFSet>(swl));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NullMaterial)