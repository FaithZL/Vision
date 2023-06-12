//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class MirrorBxDFSet : public BxDFSet {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _bxdf;

public:
    MirrorBxDFSet(const SP<Fresnel> &fresnel, MicrofacetReflection bxdf)
        : _fresnel(fresnel), _bxdf(std::move(bxdf)) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, _fresnel->clone());
    }

    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _bxdf.sample(wo, sampler, _fresnel->clone());
    }

    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Uint flag,
                                             Sampler *sampler) const noexcept override {
        return _bxdf.sample_wi(wo, sampler->next_2d(), _fresnel->clone());
    }
};

class MirrorMaterial : public Material {
private:
    Slot _color{};
    Slot _roughness{};
    bool _remapping_roughness{false};

public:
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc), _color(_scene->create_slot(desc.slot("color", make_float3(1.f), Albedo))),
          _roughness(_scene->create_slot(desc.slot("roughness", make_float2(0.0001f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(false)) {
        init_slot_cursor(&_color, 2);
    }

    [[nodiscard]] BSDF compute_BSDF(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = _color.eval_albedo_spectrum(it, swl).sample;
        Float2 alpha = _roughness.evaluate(it, swl).as_vec2();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelNoOp>(swl, pipeline());
        MicrofacetReflection bxdf(kr, swl, microfacet);
        return BSDF(it, make_unique<MirrorBxDFSet>(fresnel, ocarina::move(bxdf)));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MirrorMaterial)