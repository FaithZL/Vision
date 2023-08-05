//
// Created by Zero on 22/10/2022.
//

#include "shape.h"

#include <utility>
#include "base/mgr/scene.h"
#include "base/mgr/geometry.h"
#include "base/mgr/mesh_pool.h"

namespace vision {

ShapeInstance::ShapeInstance(SP<vision::Mesh> mesh)
    : _mesh(ocarina::move(mesh)) {}

ShapeInstance::ShapeInstance(vision::Mesh mesh)
    : _mesh(MeshPool::instance().register_(ocarina::move(mesh))) {}

void ShapeInstance::fill_geometry(vision::Geometry &data) const noexcept {
    data.accept(_mesh->vertices, _mesh->triangles, _handle);
}

vector<float> ShapeInstance::surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : _mesh->triangles) {
        float3 v0 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.i].position());
        float3 v1 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.j].position());
        float3 v2 = transform_point<H>(_handle.o2w, _mesh->vertices[tri.k].position());
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

Box3f ShapeInstance::compute_aabb() const noexcept {
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

ShapeGroup::ShapeGroup(const vision::ShapeDesc &desc)
    : Node(desc) {
    _material.name = desc["material"].as_string();
}

void ShapeGroup::add_instance(const vision::ShapeInstance &instance) noexcept {
    _instances.push_back(instance);
}

void ShapeGroup::add_instances(const vector<vision::ShapeInstance> &instances) noexcept {
    for (const auto &instance : instances) {
        add_instance(instance);
    }
}

void ShapeGroup::post_init(const vision::ShapeDesc &desc) {
    string mat_name = desc["material"].as_string();
    if (desc.contains("medium")) {
        string inside = desc["medium"]["inside"].as_string();
        string outside = desc["medium"]["outside"].as_string();
        for_each([&](ShapeInstance &instance, uint i) {
            instance.set_inside_name(inside);
            instance.set_outside_name(outside);
            instance.set_material_name(mat_name);
            instance.set_o2w(desc.o2w.mat);
            instance.init_aabb();
            aabb.extend(instance.aabb);
        });
    } else {
        for_each([&](ShapeInstance &instance, uint i) {
            instance.set_inside(scene().global_medium());
            instance.set_outside(scene().global_medium());
            instance.set_material_name(mat_name);
            instance.set_o2w(desc.o2w.mat);
            instance.init_aabb();
            aabb.extend(instance.aabb);
        });
    }
}

}// namespace vision