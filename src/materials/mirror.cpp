//
// Created by Zero on 28/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class MirrorBSDF : public BSDF {
private:
    MicrofacetReflection _bxdf;

public:
    explicit MirrorBSDF(const SurfaceInteraction &si, Float3 kr, Float ax, Float ay)
        : BSDF(si), _bxdf(kr, ax, ay, FresnelType::NoOp, GGX) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u,
                                          Uchar flag) const noexcept override {
        return _bxdf.sample(wo, u);
    }
};

class MirrorMaterial : public Material {
private:
    Texture *_color{};
    Texture *_roughness{};

public:
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc), _color(desc.scene->load<Texture>(desc.color)),
          _roughness(desc.scene->load<Texture>(desc.roughness)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        Float3 kr = _color ? _color->eval(si).xyz() : make_float3(0.f);
        Float2 alpha = _roughness ? _roughness->eval(si).xy() : make_float2(0.001f);
        auto ret = make_unique<MirrorBSDF>(si, kr, alpha.x, alpha.y);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MirrorMaterial)