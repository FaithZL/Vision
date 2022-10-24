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

void Mesh::fill_device_data(DeviceData &data, size_t *inst_id) const noexcept {

    data.vertices.append(vertices);
    data.triangles.append(triangles);
    data.add_mesh(data.vertices.device(), data.triangles.device());
    *inst_id += 1;
}

}// namespace vision