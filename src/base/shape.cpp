//
// Created by Zero on 22/10/2022.
//

#include "shape.h"
#include "bake_utlis.h"
#include <utility>
#include "base/scattering/material.h"
#include "math/transform.h"
#include "base/mgr/mesh_registry.h"

namespace vision {

ShapeInstance::ShapeInstance(SP<vision::Mesh> mesh)
    : mesh_(ocarina::move(mesh)) {}

ShapeInstance::ShapeInstance(vision::Mesh mesh)
    : mesh_(MeshRegistry::instance().register_(ocarina::move(mesh))) {}

void ShapeInstance::fill_mesh_id() noexcept {
    handle_.mesh_id = mesh_->index();
}

bool ShapeInstance::render_UI(ocarina::Widgets *widgets) noexcept {
    if (has_material()) {
        material()->render_UI(widgets);
    }
    if (has_emission()) {
        emission()->render_UI(widgets);
    }
    if (has_inside()) {
        widgets->text("inside medium");
        inside()->render_UI(widgets);
    }
    if (has_outside()) {
        widgets->text("outside medium");
        outside()->render_UI(widgets);
    }
    widgets->use_tree("geometry detail", [&] {
        render_sub_UI(widgets);
    });
    return true;
}

void ShapeInstance::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    float4x4 o2w = handle_.o2w();
    float3 t;
    quaternion r;
    float3 s;
    decompose(o2w, addressof(t), addressof(r), addressof(s));
    widgets->drag_float3("position", addressof(t), 0.1f);
    widgets->drag_float3("scale", addressof(s), 0.1f);
    float4x4 new_o2w = translation(t) * make_float4x4(r.to_float3x3()) * scale(s);
}

vector<float> ShapeInstance::surface_areas() const noexcept {
    vector<float> ret;
    for (const Triangle &tri : mesh_->triangles()) {
        float3 v0 = transform_point<H>(handle_.o2w(), mesh_->vertices()[tri.i].position());
        float3 v1 = transform_point<H>(handle_.o2w(), mesh_->vertices()[tri.j].position());
        float3 v2 = transform_point<H>(handle_.o2w(), mesh_->vertices()[tri.k].position());
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

Box3f ShapeInstance::compute_aabb() const noexcept {
    Box3f box;
    for (const Triangle &tri : mesh_->triangles()) {
        float3 v0 = transform_point<H>(handle_.o2w(), mesh_->vertices()[tri.i].position());
        float3 v1 = transform_point<H>(handle_.o2w(), mesh_->vertices()[tri.j].position());
        float3 v2 = transform_point<H>(handle_.o2w(), mesh_->vertices()[tri.k].position());
        box.extend(v0);
        box.extend(v1);
        box.extend(v2);
    }
    return box;
}

GPUMesh::GPUMesh(ocarina::uint vert_num, ocarina::uint tri_num) noexcept {
    update_data(vert_num, tri_num);
}

GPUMesh::GPUMesh(vision::GPUMesh &&other) noexcept
    : triangle_buffer_(std::move(other.triangle_buffer_)),
      vertex_buffer_(std::move(other.vertex_buffer_)),
      index_(other.index_) {}

GPUMesh &GPUMesh::operator=(vision::GPUMesh &&other) noexcept {
    vertex_buffer_ = std::move(other.vertex_buffer_);
    triangle_buffer_ = std::move(other.triangle_buffer_);
    index_ = other.index_;
    return *this;
}

GPUMesh::GPUMesh(const vector<Vertex> &vert,
                 const vector<Triangle> &triangles) noexcept
    : GPUMesh(vert.size(), triangles.size()) {}

void GPUMesh::update_data(ocarina::uint vert_num, ocarina::uint tri_num) noexcept {
    auto ppl = Global::instance().pipeline();
    auto v_buffer = ppl->device().create_buffer<Vertex>(vert_num, "GPUMesh::vertex_buffer_");
    auto t_buffer = ppl->device().create_buffer<Triangle>(tri_num, "GPUMesh::triangle_buffer_");
    vertex_buffer_.set_bindless_array(ppl->bindless_array());
    triangle_buffer_.set_bindless_array(ppl->bindless_array());
    vertex_buffer_.update_buffer(std::move(v_buffer));
    triangle_buffer_.update_buffer(std::move(t_buffer));
}

void GPUMesh::upload_vertices_immediately(const vector<vision::Vertex> &vertices) noexcept {
    vertex_buffer_.upload_immediately(vertices.data());
}

void GPUMesh::upload_triangles_immediately(const vector<vision::Triangle> &triangles) noexcept {
    triangle_buffer_.upload_immediately(triangles.data());
}

BufferUploadCommand *GPUMesh::upload_triangles(const vector<vision::Triangle> &triangles) noexcept {
    return triangle_buffer_.upload(triangles.data());
}

BufferUploadCommand *GPUMesh::upload_vertices(const vector<vision::Vertex> &vertices) noexcept {
    return vertex_buffer_.upload(vertices.data());
}

uint64_t Mesh::compute_hash() const noexcept {
    uint64_t ret = Hash64::default_seed;
    for (Vertex vertex : vertices_) {
        ret = hash64(vertex, ret);
    }
    for (Triangle triangle : triangles_) {
        ret = hash64(triangle, ret);
    }
    return ret;
}

void Mesh::setup_lightmap_uv(const UnwrapperResult &result) {
    resolution_ = make_uint2(result.width, result.height);
    const UnwrapperMesh &u_mesh = result.meshes[0];
    vector<Vertex> vertices;
    vertices.reserve(u_mesh.vertices.size());
    for (auto &vert : u_mesh.vertices) {
        Vertex vertex = vertices_[vert.xref];
        vertex.set_lightmap_uv(vert.uv);
        vertices.push_back(vertex);
    }
    set_vertices(ocarina::move(vertices));
    set_triangles(u_mesh.triangles);
    has_lightmap_uv_ = true;
}

void Mesh::normalize_lightmap_uv() noexcept {
    if (normalized_) {
        return;
    }
    for (Vertex &vertex : vertices_) {
        vertex.set_lightmap_uv(vertex.lightmap_uv() / make_float2(resolution_));
    }
    normalized_ = true;
}

void Mesh::upload_immediately() noexcept {
    upload_triangles_immediately(triangles_);
    upload_vertices_immediately(vertices_);
}

CommandList Mesh::upload() noexcept {
    CommandList ret;
    ret << upload_triangles(triangles_);
    ret << upload_vertices(vertices_);
    return ret;
}

void Mesh::update_data() noexcept {
    GPUMesh::update_data(vertices_.size(), triangles_.size());
}

Box3f Mesh::compute_aabb() const noexcept {
    Box3f box;
    for (const Triangle &tri : triangles_) {
        float3 v0 = vertices_[tri.i].position();
        float3 v1 = vertices_[tri.j].position();
        float3 v2 = vertices_[tri.k].position();
        box.extend(v0);
        box.extend(v1);
        box.extend(v2);
    }
    return box;
}

float2 Mesh::lightmap_uv_unnormalized(uint index) const noexcept {
    float2 ret = vertices_[index].lightmap_uv();
    if (normalized_) {
        ret *= make_float2(resolution_);
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
    for (const Triangle &tri : triangles_) {
        float3 v0 = vertices_[tri.i].position();
        float3 v1 = vertices_[tri.j].position();
        float3 v2 = vertices_[tri.k].position();
        ret.push_back(triangle_area(v0, v1, v2));
    }
    return ret;
}

ShapeGroup::ShapeGroup(const vision::ShapeDesc &desc)
    : Node(desc) {
    material_.name = desc["material"].as_string();
}

ShapeGroup::ShapeGroup(vision::ShapeInstance inst) {
    inst.init_aabb();
    aabb.extend(inst.aabb);
    inst.set_name(ocarina::format("{}_0", name()));

    instances_.push_back(make_shared<ShapeInstance>(inst));
}

void ShapeGroup::add_instance(const vision::ShapeInstance &instance) noexcept {
    instances_.push_back(make_shared<ShapeInstance>(instance));
}

void ShapeGroup::add_instances(vector<vision::ShapeInstance> instances) noexcept {
    for (const auto &instance : instances) {
        add_instance(instance);
    }
}

void ShapeGroup::post_init(const vision::ShapeDesc &desc) {
    string mat_name = desc["material"].as_string();
    if (desc.contains("medium")) {
        string inside = desc["medium"]["inside"].as_string();
        string outside = desc["medium"]["outside"].as_string();
        for_each([&](SP<ShapeInstance> instance, uint i) {
            instance->set_inside_name(inside);
            instance->set_outside_name(outside);
            instance->set_material_name(mat_name);
            instance->set_o2w(desc.o2w.mat);
            instance->init_aabb();
            instance->set_name(ocarina::format("{}_{}", name(), i));
            aabb.extend(instance->aabb);
        });
    } else {
        for_each([&](SP<ShapeInstance> instance, uint i) {
            instance->set_inside(scene().global_medium());
            instance->set_outside(scene().global_medium());
            instance->set_material_name(mat_name);
            instance->set_o2w(desc.o2w.mat);
            instance->init_aabb();
            instance->set_name(ocarina::format("{}_{}", name(), i));
            aabb.extend(instance->aabb);
        });
    }
}
}// namespace vision