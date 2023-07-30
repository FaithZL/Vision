//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "base/mgr/scene.h"
#include "base/mgr/geometry.h"

namespace vision {

Shape::Shape(const ShapeDesc &desc)
    : Node(desc),
      _factor(desc["factor"].as_float(1.f)) {
    material.name = desc["material"].as_string();
//    inside.name = desc.
    _handle.inside_medium = desc.inside_medium.id;
    _handle.outside_medium = desc.outside_medium.id;
    _handle.o2w = desc.o2w.mat;
}

void Shape::load_light(const vision::LightDesc &desc) noexcept {
    if (desc.valid()) {
        emission.object = scene().load_light(desc);
    }
}

uint Shape::lightmap_size() const noexcept {
    vector<float> areas = ref_surface_areas();
    float area = std::accumulate(areas.begin(), areas.end(), 0.f);
    uint ret = area * _factor * 20;
    return ret;
}

Mesh::Mesh(const ShapeDesc &desc) : Shape(desc) {}

void Mesh::fill_geometry(Geometry &data) const noexcept {
    data.accept(vertices, triangles, _handle);
}

uint64_t Mesh::_compute_hash() const noexcept {
    uint64_t ret = Hash64::default_seed;
    for (Vertex vertex : vertices) {
        ret = hash64(vertex, ret);
    }
    for (Triangle triangle : triangles) {
        ret = hash64(triangle, ret);
    }
    return ret;
}

Box3f Mesh::compute_aabb() const noexcept {
    Box3f box;
    for (const Triangle &tri : triangles) {
        float3 v0 = transform_point<H>(_handle.o2w, vertices[tri.i].position());
        float3 v1 = transform_point<H>(_handle.o2w, vertices[tri.j].position());
        float3 v2 = transform_point<H>(_handle.o2w, vertices[tri.k].position());
        box.extend(v0);
        box.extend(v1);
        box.extend(v2);
    }
    return box;
}

vector<float> Mesh::surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : triangles) {
        float3 v0 = transform_point<H>(_handle.o2w, vertices[tri.i].position());
        float3 v1 = transform_point<H>(_handle.o2w, vertices[tri.j].position());
        float3 v2 = transform_point<H>(_handle.o2w, vertices[tri.k].position());
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

vector<float> Mesh::ref_surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : triangles) {
        float3 v0 = vertices[tri.i].position();
        float3 v1 = vertices[tri.j].position();
        float3 v2 = vertices[tri.k].position();
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

}// namespace vision