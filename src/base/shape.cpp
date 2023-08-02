//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "base/mgr/scene.h"
#include "base/mgr/geometry.h"

namespace vision {

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
    : Node(desc) {
    _material.name = desc["material"].as_string();
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
            _aabb.extend(instance.aabb);
        });
    } else {
        for_each([&](Instance &instance, uint i) {
            instance.set_inside(scene().global_medium());
            instance.set_outside(scene().global_medium());
            instance.set_material_name(mat_name);
            instance.set_o2w(desc.o2w.mat);
            instance.init_aabb();
            _aabb.extend(instance.aabb);
        });
    }
}

}// namespace vision