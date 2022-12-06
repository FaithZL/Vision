//
// Created by Zero on 09/09/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class MatteBSDF : public BSDF {
private:
    LambertReflection _bxdf;

public:
    explicit MatteBSDF(const Interaction &si, const Float3 &kr)
        : BSDF(si), _bxdf(kr) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, nullptr);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u,
                                          Uchar flag) const noexcept override {
        return _bxdf.sample(wo, u, nullptr);
    }
};

class MatteMaterial : public Material {
private:
    const Texture *_color{};

public:
    explicit MatteMaterial(const MaterialDesc &desc)
        : Material(desc), _color(desc.scene->load<Texture>(desc.color)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si) const noexcept override {
        Float3 kr = Texture::eval(_color, si).xyz();
        return make_unique<MatteBSDF>(si, kr);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MatteMaterial)