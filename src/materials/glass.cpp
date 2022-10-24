//
// Created by Zero on 05/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {
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
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GlassMaterial)