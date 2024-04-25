//
// Created by Zero on 2023/7/25.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class PbrBxDFSet : public BxDFSet {
private:
    DCSP<BxDF> bxdf_;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(bxdf_->type_hash());
    }

public:
    PbrBxDFSet(const SampledSpectrum &kr, const SampledWavelengths &swl)
        : bxdf_(std::make_shared<LambertReflection>(kr, swl)) {}
    // clang-format off
    VS_MAKE_BxDFSet_ASSIGNMENT(PbrBxDFSet)
    // clang-format on
    [[nodiscard]] SampledSpectrum albedo(const Float3 &wo) const noexcept override { return bxdf_->albedo(wo); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, MaterialEvalMode mode, Uint flag) const noexcept override {
        return bxdf_->safe_evaluate(wo, wi, nullptr, mode);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, SamplerImpl *sampler) const noexcept override {
        return bxdf_->sample(wo, sampler, nullptr);
    }
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Uint flag, SamplerImpl *sampler) const noexcept override {
        return bxdf_->sample_wi(wo, sampler->next_2d(), nullptr);
    }
};

class PbrMaterial : public Material {
private:
    VS_MAKE_SLOT(color)
    VS_MAKE_SLOT(spec)
    VS_MAKE_SLOT(roughness)
    VS_MAKE_SLOT(metallic)

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<PbrBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    explicit PbrMaterial(const MaterialDesc &desc)
        : Material(desc) {
        color_.set(Slot::create_slot(desc.slot("color", make_float3(0.5f), Albedo)));
        init_slot_cursor(&color_, 1);
    }
    VS_MAKE_PLUGIN_NAME_FUNC

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = color_.eval_albedo_spectrum(it, swl).sample;
        return make_unique<PbrBxDFSet>(kr, swl);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PbrMaterial)