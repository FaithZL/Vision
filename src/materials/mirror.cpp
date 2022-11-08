//
// Created by Zero on 28/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class MirrorMaterial : public Material {
private:
    Texture *_color{};
    Texture * _roughness{};
public:
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MirrorMaterial)