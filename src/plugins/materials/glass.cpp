//
// Created by Zero on 05/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class GlassMaterial : public Material {
private:
    const Texture *_color{};
    const Texture *_ior{};
    const Texture *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit GlassMaterial(const MaterialDesc &desc)
        : Material(desc),
          _color(desc.scene->load<Texture>(desc.color)),
          _ior(desc.scene->load<Texture>(desc.ior)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _remapping_roughness(desc.remapping_roughness) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si) const noexcept override {
        Float3 color = Texture::eval(_color, si).xyz();
        Float ior = Texture::eval(_ior, si, 1.5f).x;
        Float2 alpha = Texture::eval(_roughness, si, 1.f).xy();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(ior);
        MicrofacetReflection refl(make_float3(1.f), microfacet);
        MicrofacetTransmission trans(color, microfacet);
        return make_unique<DielectricBSDF>(si, fresnel, move(refl), move(trans));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GlassMaterial)