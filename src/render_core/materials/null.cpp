//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class NullBSDF : public BSDF {
public:
    NullBSDF(const Interaction &it,const SampledWavelengths &swl)
        : BSDF(it, swl) {}
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept {
        ScatterEval ret{swl.dimension()};
        ret.f = {swl.dimension(), 0.f};
        ret.pdf = 1.f;
        return ret;
    }

    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
        BSDFSample ret{swl.dimension()};
        return ret;
    }
};

class NullMaterial : public Material {
public:
    explicit NullMaterial(const MaterialDesc &desc)
        : Material(desc) {}

    [[nodiscard]] UP<BSDF> compute_BSDF(const Interaction &it,
                                        const SampledWavelengths &swl) const noexcept override {
        return make_unique<NullBSDF>(it, swl);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NullMaterial)