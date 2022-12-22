//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class MirrorBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _bxdf;

public:
    MirrorBSDF(const Interaction &si, const SP<Fresnel> &fresnel, MicrofacetReflection bxdf)
        : BSDF(si), _fresnel(fresnel), _bxdf(std::move(bxdf)) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, _fresnel->clone());
    }

    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept override {
        return _bxdf.sample(wo, sampler, _fresnel->clone());
    }
};

class MirrorMaterial : public Material {
private:
    const Texture *_color{};
    const Texture *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc), _color(desc.scene->load<Texture>(desc.color)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _remapping_roughness(desc.remapping_roughness) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si) const noexcept override {
        Float3 kr = Texture::eval(_color, si).xyz();
        Float2 alpha = Texture::eval(_roughness, si, 0.0001f).xy();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelNoOp>();
        MicrofacetReflection bxdf(kr, microfacet);
        return make_unique<MirrorBSDF>(si, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MirrorMaterial)