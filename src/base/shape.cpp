//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "bake_utlis.h"
#include <utility>
#include "base/mgr/scene.h"
#include "base/mgr/geometry.h"
#include "base/mgr/mesh_registry.h"

namespace vision {

ShapeInstance::ShapeInstance(SP<vision::Mesh> mesh)
    : _mesh(ocarina::move(mesh)) {}

ShapeInstance::ShapeInstance(vision::Mesh mesh)
    : _mesh(MeshRegistry::instance().register_(ocarina::move(mesh))) {}

void ShapeInstance::fill_mesh_id() noexcept {
    _handle.mesh_id = _mesh->index();
}

vector<float> ShapeInstance::surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : _mesh->triangles()) {
        float3 v0 = transform_point<H>(_handle.o2w, _mesh->vertices()[tri.i].position());
        float3 v1 = transform_point<H>(_handle.o2w, _mesh->vertices()[tri.j].position());
        float3 v2 = transform_point<H>(_handle.o2w, _mesh->vertices()[tri.k].position());
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

Box3f ShapeInstance::compute_aabb() const noexcept {
    Box3f box;
    for (const Triangle &tri : _mesh->triangles()) {
        float3 v0 = transform_point<H>(_handle.o2w, _mesh->vertices()[tri.i].position());
        float3 v1 = transform_point<H>(_handle.o2w, _mesh->vertices()[tri.j].position());
        float3 v2 = transform_point<H>(_handle.o2w, _mesh->vertices()[tri.k].position());
        box.extend(v0);
        box.extend(v1);
        box.extend(v2);
    }
    return box;
}

uint64_t Mesh::_compute_hash() const noexcept {
    uint64_t ret = Hash64::default_seed;
    for (Vertex vertex : _vertices) {
        ret = hash64(vertex, ret);
    }
    for (Triangle triangle : _triangles) {
        ret = hash64(triangle, ret);
    }
    return ret;
}

void Mesh::setup_lightmap_uv(const UnwrapperResult &result) {
    _resolution = make_uint2(result.width, result.height);
    const UnwrapperMesh &u_mesh = result.meshes[0];
    vector<Vertex> vertices;
    vertices.reserve(u_mesh.vertices.size());
    for (auto &vert : u_mesh.vertices) {
        Vertex vertex = _vertices[vert.xref];
        vertex.set_lightmap_uv(vert.uv);
        vertices.push_back(vertex);
    }
    set_vertices(ocarina::move(vertices));
    set_triangles(u_mesh.triangles);
    _has_lightmap_uv = true;
}

void Mesh::normalize_lightmap_uv() noexcept {
    if (_normalized) {
        return;
    }
    for (Vertex &vertex : _vertices) {
        vertex.set_lightmap_uv(vertex.lightmap_uv() / make_float2(_resolution));
    }
    _normalized = true;
}

Box3f Mesh::compute_aabb() const noexcept {
    Box3f box;
    for (const Triangle &tri : _triangles) {
        float3 v0 = _vertices[tri.i].position();
        float3 v1 = _vertices[tri.j].position();
        float3 v2 = _vertices[tri.k].position();
        box.extend(v0);
        box.extend(v1);
        box.extend(v2);
    }
    return box;
}

float2 Mesh::lightmap_uv_unnormalized(uint index) const noexcept {
    float2 ret = _vertices[index].lightmap_uv();
    if (_normalized) {
        ret *= make_float2(_resolution);
    }
    return ret;
}

uint Mesh::lightmap_size() const noexcept {
    vector<float> areas = surface_areas();
    float area = std::accumulate(areas.begin(), areas.end(), 0.f);
    uint ret = area * 20;
    return ret;
}

vector<float> Mesh::surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : _triangles) {
        float3 v0 = _vertices[tri.i].position();
        float3 v1 = _vertices[tri.j].position();
        float3 v2 = _vertices[tri.k].position();
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

ShapeGroup::ShapeGroup(const vision::ShapeDesc &desc)
    : Node(desc) {
    _material.name = desc["material"].as_string();
}

ShapeGroup::ShapeGroup(vision::ShapeInstance inst) {
    inst.init_aabb();
    aabb.extend(inst.aabb);
    inst.set_name(ocarina::format("{}_0", name()));
    _instances.push_back(inst);
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
            instance.set_name(ocarina::format("{}_{}", name(), i));
            aabb.extend(instance.aabb);
        });
    } else {
        for_each([&](ShapeInstance &instance, uint i) {
            instance.set_inside(scene().global_medium());
            instance.set_outside(scene().global_medium());
            instance.set_material_name(mat_name);
            instance.set_o2w(desc.o2w.mat);
            instance.init_aabb();
            instance.set_name(ocarina::format("{}_{}", name(), i));
            aabb.extend(instance.aabb);
        });
    }
}

}// namespace vision