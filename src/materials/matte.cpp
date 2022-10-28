//
// Created by Zero on 09/09/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"
#include "base/bxdf.h"

namespace vision {

class MatteBSDF : public BSDF {
private:
    LambertReflection _bxdf;

public:
    MatteBSDF(const SurfaceInteraction &si, const Float3 &kr)
        : BSDF(si), _bxdf(kr) {}

    [[nodiscard]] Float PDF_(Float3 wo, Float3 wi) const noexcept override {
        return _bxdf.PDF(wo, wi);
    }

    [[nodiscard]] Float3 eval_(Float3 wo, Float3 wi) const noexcept override {
        return _bxdf.eval(wo, wi);
    }

    [[nodiscard]] BSDFSample sample_(Float3 wo, Float uc, Float2 u) const noexcept override {
        return _bxdf.sample(wo, u);
    }
};

class MatteMaterial : public Material {
private:
    Texture *_color{};

public:
    explicit MatteMaterial(const MaterialDesc &desc)
        : Material(desc), _color(desc.scene->load<Texture>(desc.color)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        return make_unique<MatteBSDF>(si, _color->eval(si).xyz());
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MatteMaterial)