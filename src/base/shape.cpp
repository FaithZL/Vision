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
    _material.name = desc["material"].as_string();
    if (scene().has_medium()) {
        init_medium(desc);
    }
    _handle.o2w = desc.o2w.mat;
}

void Shape::init_medium(const vision::ShapeDesc &desc) noexcept {
    if (desc.contains("medium")) {
        _inside.name = desc["medium"]["inside"].as_string();
        _outside.name = desc["medium"]["outside"].as_string();
    } else {
        _inside = scene().global_medium();
        _outside = scene().global_medium();
    }
}

uint Shape::lightmap_size() const noexcept {
    vector<float> areas = ref_surface_areas();
    float area = std::accumulate(areas.begin(), areas.end(), 0.f);
    uint ret = area * _factor * 20;
    return ret;
}

void Instance::fill_geometry(vision::Geometry &data) const noexcept {
    data.accept(_mesh->vertices, _mesh->triangles, _handle);
}

vector<float> Instance::surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : _mesh->triangles) {
        float3 v0 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.i].position());
        float3 v1 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.j].position());
        float3 v2 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.k].position());
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

Box3f Instance::compute_aabb() const noexcept {
    Box3f box;
    for (const Triangle &tri : _mesh->triangles) {
        float3 v0 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.i].position());
        float3 v1 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.j].position());
        float3 v2 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.k].position());
        box.extend(v0);
        box.extend(v1);
        box.extend(v2);
    }
    return box;
}

Mesh::Mesh(const ShapeDesc &desc) {}

void Mesh::fill_geometry(Geometry &data) const noexcept {
    //    data.accept(vertices, triangles, _handle);
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
        float3 v0 = vertices[tri.i].position();
        float3 v1 = vertices[tri.j].position();
        float3 v2 = vertices[tri.k].position();
        box.extend(v0);
        box.extend(v1);
        box.extend(v2);
    }
    return box;
}

uint Mesh::lightmap_size() const noexcept {
    vector<float> areas = surface_areas();
    float area = std::accumulate(areas.begin(), areas.end(), 0.f);
    uint ret = area * 20;
    return ret;
}

vector<float> Mesh::surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : triangles) {
        float3 v0 = vertices[tri.i].position();
        float3 v1 = vertices[tri.j].position();
        float3 v2 = vertices[tri.k].position();
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

Group::Group(const vision::ShapeDesc &desc)
    : Node(desc), _o2w(desc.o2w.mat) {
    _material.name = desc["material"].as_string();
    if (scene().has_medium()) {
        init_medium(desc);
    }
}

void Group::post_init(const vision::ShapeDesc &desc) {
    string mat_name = desc["material"].as_string();
    if (desc.contains("medium")) {
        string inside = desc["medium"]["inside"].as_string();
        string outside = desc["medium"]["outside"].as_string();
        for_each([&](Instance &instance, uint i) {
            instance.set_inside_name(inside);
            instance.set_outside_name(outside);
            instance.set_material_name(mat_name);
            instance.set_o2w(desc.o2w.mat);
            instance.init_aabb();
        });
    } else {
        for_each([&](Instance &instance, uint i) {
            instance.set_inside(scene().global_medium());
            instance.set_outside(scene().global_medium());
            instance.set_material_name(mat_name);
            instance.set_o2w(desc.o2w.mat);
            instance.init_aabb();
        });
    }
}

void Group::init_medium(const vision::ShapeDesc &desc) noexcept {
    if (desc.contains("medium")) {
        _inside.name = desc["medium"]["inside"].as_string();
        _outside.name = desc["medium"]["outside"].as_string();
    } else {
        _inside = scene().global_medium();
        _outside = scene().global_medium();
    }
}

}// namespace vision