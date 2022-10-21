//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "scene.h"

namespace vision {
Shape::Shape(const ShapeDesc *desc) : Node(desc) {
    o2w = desc->o2w.mat;
    if (desc->emission.valid_emission()) {
        emission = desc->scene->load<Texture>(&desc->emission);
    }
}
}// namespace vision