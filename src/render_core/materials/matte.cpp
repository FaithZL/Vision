//
// Created by Zero on 09/09/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

class MatteBSDF : public BSDF {
private:
    LambertReflection _bxdf;

public:
    explicit MatteBSDF(const Interaction &si, const SampledSpectrum &kr, const SampledWavelengths &swl)
        : BSDF(si, swl), _bxdf(kr, swl) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, nullptr);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _bxdf.sample(wo, sampler, nullptr);
    }
};

class MatteMaterial : public Material {
private:
    Slot _color{};

public:
    explicit MatteMaterial(const MaterialDesc &desc)
        : Material(desc), _color(_scene->create_slot(desc.slot("color", make_float3(0.5f), Albedo))) {}

    void prepare() noexcept override {
        RenderPipeline *rp = render_pipeline();

    }

    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return _color.type_hash();
    }

    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return _color.hash();
    }

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = _color.eval_albedo_spectrum(si, swl).sample;
        return make_unique<MatteBSDF>(si, kr, swl);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MatteMaterial)