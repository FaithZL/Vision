//
// Created by Zero on 06/11/2022.
//

#include "base/scattering/material.h"
#include "base/mgr/scene.h"

namespace vision {

class NullMaterial : public Material {
public:
    explicit NullMaterial(const MaterialDesc &desc)
        : Material(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NullMaterial)