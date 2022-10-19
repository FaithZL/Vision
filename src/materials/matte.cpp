//
// Created by Zero on 09/09/2022.
//

#include "base/material.h"

namespace vision {
class MatteMaterial : public Material {
public:
    explicit MatteMaterial(const MaterialDesc *desc) : Material(desc) {}
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MatteMaterial)