//
// Created by Zero on 05/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class GlassBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _refl;
    MicrofacetTransmission _trans;

public:
    GlassBSDF(const SurfaceInteraction &si,
              const SP<Fresnel> &fresnel,
                  MicrofacetReflection refl,
              MicrofacetTransmission trans)
        : BSDF(si), _fresnel(fresnel), _refl(move(refl)), _trans(move(trans)) {}

    [[nodiscard]] Float3 albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        BSDFEval ret;
        auto fresnel = _fresnel->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        $if(same_hemisphere(wo, wi)) {
            ret = _refl.evaluate(wo, wi, fresnel);
        }
        $else {
            ret = _trans.evaluate(wo, wi, fresnel);
        };
        return ret;
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u, Uchar flag) const noexcept override {
        BSDFSample ret;
        auto fresnel = _fresnel->clone();
        Float cos_theta_o = cos_theta(wo);
        fresnel->correct_eta(cos_theta_o);
        Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
        $if(uc < fr) {
            ret = _refl.sample(wo, u, fresnel);
            ret.eval.pdf *= fr;
        }
        $else {
            ret = _trans.sample(wo, u, fresnel);
            ret.eval.pdf *= 1 - fr;
        };
        return ret;
    }
};

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

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        Float3 color = eval_tex(_color, si).xyz();
        Float ior = eval_tex(_ior, si, 1.5f).x;
        Float2 alpha = eval_tex(_roughness, si, 0.0001f).xy();
        auto microfacet = make_shared<Microfacet<D>>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelDielectric>(ior);
        MicrofacetReflection refl(color, microfacet);
        MicrofacetTransmission trans(color, microfacet);
        return make_unique<GlassBSDF>(si, fresnel, move(refl), move(trans));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GlassMaterial)