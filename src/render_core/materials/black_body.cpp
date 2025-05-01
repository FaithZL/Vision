//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class BlackBodyLobe : public Lobe {
private:
    const SampledWavelengths *swl_{nullptr};

public:
    explicit BlackBodyLobe(const SampledWavelengths &swl) : swl_(&swl) {}
    [[nodiscard]] Uint flag() const noexcept override { return BxDFFlag::Diffuse; }
    [[nodiscard]] ScatterEval evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode, const Uint &flag,
                                             TransportMode tm) const noexcept override;
    [[nodiscard]] const SampledWavelengths *swl() const override;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                          TransportMode tm) const noexcept override;
    [[nodiscard]] SampledSpectrum albedo(const Float &cos_theta) const noexcept override {
        return {swl_->dimension(), 0.f};
    }
    VS_MAKE_LOBE_ASSIGNMENT(BlackBodyLobe)
};

const SampledWavelengths *BlackBodyLobe::swl() const {
    return swl_;
}

ScatterEval BlackBodyLobe::evaluate_local_impl(const Float3 &wo, const Float3 &wi,
                                          MaterialEvalMode mode, const Uint &flag,
                                          TransportMode tm) const noexcept {
    ScatterEval ret{*swl_};
    ret.f = {swl_->dimension(), 0.f};
    ret.pdfs = 1.f;
    return ret;
}

BSDFSample BlackBodyLobe::sample_local(const Float3 &wo, const Uint &flag, TSampler &sampler,
                                       TransportMode tm) const noexcept {
    BSDFSample ret{*swl_};
    ret.eval.pdfs = 1.f;
    /// Avoid sample discarding due to hemispherical check
    ret.eval.flags = BxDFFlag::DiffRefl;
    ret.wi = wo;
    return ret;
}

class BlackBodyMaterial : public Material {
protected:
    VS_MAKE_MATERIAL_EVALUATOR(BlackBodyLobe)

public:
    [[nodiscard]] UP<Lobe> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        return make_unique<BlackBodyLobe>(swl);
    }
    [[nodiscard]] bool enable_delta() const noexcept override { return false; }
    explicit BlackBodyMaterial(const MaterialDesc &desc)
        : Material(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BlackBodyMaterial)