//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class BlackBodyBxDFSet : public BxDFSet {
private:
    const SampledWavelengths *swl_{nullptr};

public:
    explicit BlackBodyBxDFSet(const SampledWavelengths &swl) : swl_(&swl) {}
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag) const noexcept override {
        ScatterEval ret{swl_->dimension()};
        ret.f = {swl_->dimension(), 0.f};
        ret.pdfs =1.f;
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler) const noexcept override {
        BSDFSample ret{swl_->dimension()};
        ret.eval.pdfs =1.f;
        /// Avoid sample discarding due to hemispherical check
        ret.eval.flags = BxDFFlag::DiffRefl;
        ret.wi = wo;
        return ret;
    }
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override {
        return {swl_->dimension(), 0.f};
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
    [[nodiscard]] bool enable_delta() const noexcept override { return false; }
    explicit BlackBodyMaterial(const MaterialDesc &desc)
        : Material(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BlackBodyMaterial)