//
// Created by Zero on 09/09/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"
#include "base/bxdf.h"

namespace vision {


class MatteMaterial : public Material {
private:
    Texture *_color{};

public:
    explicit MatteMaterial(const MaterialDesc &desc)
        : Material(desc), _color(desc.scene->load<Texture>(desc.color)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        auto ret = make_unique<BSDF>(si);
        Float3 kr = _color ? _color->eval(si).xyz() : make_float3(0.f);
        ret->emplace_bxdf<LambertReflection>(kr);
        return ret;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MatteMaterial)