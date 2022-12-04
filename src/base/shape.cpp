//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "scene.h"
#include "geometry.h"

namespace vision {

Shape::Shape(const ShapeDesc &desc)
    : Node(desc) {
    handle.mat_id = desc.material.id;
    handle.inside_medium = desc.inside_medium.id;
    handle.outside_medium = desc.outside_medium.id;
    handle.o2w = desc.o2w.mat;
    if (desc.emission.valid()) {
        handle.light_id = desc.scene->light_num();
        emission = desc.scene->load_light(desc.emission);
    }
}

Mesh::Mesh(const ShapeDesc &desc) : Shape(desc) {}

void Mesh::fill_geometry(Geometry &data) const noexcept {
    data.accept(vertices, triangles, handle);
}

vector<float> Mesh::surface_area() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : triangles) {
        float3 v0 = transform_point<H>(handle.o2w, vertices[tri.i].position());
        float3 v1 = transform_point<H>(handle.o2w, vertices[tri.j].position());
        float3 v2 = transform_point<H>(handle.o2w, vertices[tri.k].position());
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

}// namespace vision