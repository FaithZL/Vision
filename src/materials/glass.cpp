//
// Created by Zero on 05/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class GlassBSDF : public BSDF {
private:
    MicrofacetReflection _refl;
    MicrofacetTransmission _trans;

public:
    GlassBSDF(const SurfaceInteraction &si,
              MicrofacetReflection refl,
              MicrofacetTransmission trans)
        : BSDF(si), _refl(move(refl)), _trans(move(trans)) {}

    [[nodiscard]] Float3 albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        BSDFEval ret;
        $if(same_hemisphere(wo, wi)) {
            ret = _refl.evaluate(wo, wi);
        } $else {
            ret = _trans.evaluate(wo, wi);
        };
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept override {
        BSDFSample ret;
        Float cos_theta_o = cos_theta(wo);
        return ret;
    }
};

class GlassMaterial : public Material {
private:
    Texture *_color{};
    Texture *_ior{};
    Texture *_roughness{};

public:
    explicit GlassMaterial(const MaterialDesc &desc)
        : Material(desc),
          _color(desc.scene->load<Texture>(desc.color)),
          _ior(desc.scene->load<Texture>(desc.ior)),
          _roughness(desc.scene->load<Texture>(desc.roughness)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        Float3 color = _color ? _color->eval(si).xyz() : make_float3(0.f);
        Float ior = _ior ? _ior->eval(si).x : 1.5f;
        Float2 alpha = _roughness ? _roughness->eval(si).xy() : make_float2(0.001f);
        auto microfacet = make_shared<Microfacet<D>>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(ior);
        MicrofacetReflection refl(color, microfacet, fresnel);
        MicrofacetTransmission trans(color, microfacet, fresnel);
        return make_unique<GlassBSDF>(si, move(refl), move(trans));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GlassMaterial)