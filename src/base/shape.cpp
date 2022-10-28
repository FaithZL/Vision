//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "scene.h"
#include "device_data.h"

namespace vision {

Shape::Shape(const ShapeDesc &desc) : Node(desc) {
    o2w = desc.o2w.mat;
}

Mesh::Mesh(const ShapeDesc &desc) : Shape(desc) {
    if (desc.emission.valid()) {
        light_id = desc.scene->light_num();
        emission = desc.scene->load_light(desc.emission);
    }
}

void Mesh::fill_device_data(DeviceData &data) const noexcept {
    data.accept(vertices, triangles, o2w);
}

vector<float> Mesh::surface_area() const noexcept {
    vector<float> ret;

    return ret;
}

}// namespace vision