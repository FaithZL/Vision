//
// Created by Zero on 2023/7/25.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class PbrBxDFSet : public BxDFSet {
private:
    SP<BxDF> _bxdf;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_bxdf->type_hash());
    }

public:
    PbrBxDFSet(const SampledSpectrum &kr, const SampledWavelengths &swl)
        : _bxdf(std::make_shared<LambertReflection>(kr, swl)) {}
    // clang-format off
    VS_MAKE_BxDFSet_ASSIGNMENT(PbrBxDFSet)
    PbrBxDFSet &operator=(const PbrBxDFSet &other) noexcept {
        *_bxdf = *other._bxdf;
        return *this;
    }
    // clang-format on
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _bxdf->albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _bxdf->safe_evaluate(wo, wi, nullptr);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _bxdf->sample(wo, sampler, nullptr);
    }
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _bxdf->sample_wi(wo, sampler->next_2d(), nullptr);
    }
};

class PbrMaterial : public Material {
private:
    Slot _color{};
    Slot _spec{};
    Slot _roughness{};
    Slot _metallic{};

protected:
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,
                          const SampledWavelengths &swl) const noexcept override {
        evaluator.link(ocarina::dynamic_unique_pointer_cast<PbrBxDFSet>(create_lobe_set(it, swl)));
    }

public:
    explicit PbrMaterial(const MaterialDesc &desc)
        : Material(desc), _color(scene().create_slot(desc.slot("color", make_float3(0.5f), Albedo))) {
        init_slot_cursor(&_color, 1);
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

    [[nodiscard]] UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = _color.eval_albedo_spectrum(it, swl).sample;
        return make_unique<PbrBxDFSet>(kr, swl);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PbrMaterial)