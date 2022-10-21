//
// Created by Zero on 09/09/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "core/scene.h"

namespace vision {
class MatteMaterial : public Material {
private:
    Texture *_color{};

public:
    explicit MatteMaterial(const MaterialDesc *desc)
        : Material(desc), _color(desc->scene->load<Texture>(&desc->color)) {}
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MatteMaterial)