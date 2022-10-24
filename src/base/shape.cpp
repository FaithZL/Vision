//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "scene.h"

namespace vision {

Shape::Shape(const ShapeDesc &desc) : Node(desc) {
    o2w = desc.o2w.mat;
}

Mesh::Mesh(const ShapeDesc &desc) : Shape(desc) {
    if (desc.emission.valid()) {
        emission = desc.scene->load_light(desc.emission);
    }
}

void Mesh::fill_render_data(RenderData &data, size_t *inst_id) const noexcept {

}

}// namespace vision