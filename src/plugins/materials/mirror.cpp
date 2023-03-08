//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class MirrorBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _bxdf;

public:
    MirrorBSDF(const Interaction &si, const SP<Fresnel> &fresnel, MicrofacetReflection bxdf)
        : BSDF(si, bxdf.swl()), _fresnel(fresnel), _bxdf(std::move(bxdf)) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, _fresnel->clone());
    }

    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept override {
        return _bxdf.sample(wo, sampler, _fresnel->clone());
    }
};

class MirrorMaterial : public Material {
private:
    const ShaderNode *_color{};
    const ShaderNode *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc), _color(_scene->load_shader_node(desc.attr("color", make_float3(1.f), Albedo))),
          _roughness(_scene->load_shader_node(desc.attr("roughness", make_float2(0.001f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(false)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = ShaderNode::eval_albedo_spectrum(_color, si, swl).sample;
        Float2 alpha = ShaderNode::eval(_roughness, si, 0.0001f).xy();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelNoOp>(swl,render_pipeline());
        MicrofacetReflection bxdf(kr, swl, microfacet);
        return make_unique<MirrorBSDF>(si, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MirrorMaterial)