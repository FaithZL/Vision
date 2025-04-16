//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/stl.h"
#include "math/basic_types.h"
#include "dsl/rtx_type.h"
#include "node.h"
#include "scattering/medium.h"
#include "math/box.h"
#include "core/vs_header.h"
#include "base/scattering/material.h"
#include "base/illumination/light.h"

namespace vision {

struct Geometry;

struct InstanceData {
    // todo compress unsigned int data
    uint light_id{InvalidUI32};
    uint mat_id{InvalidUI32};
    uint lightmap_id{InvalidUI32};
    uint mesh_id{InvalidUI32};
    uint inside_medium{InvalidUI32};
    uint outside_medium{InvalidUI32};
    float3x4 o2w_transposed;
    [[nodiscard]] auto o2w() const noexcept {
        return make_float4x4(transpose(o2w_transposed));
    }
};

}// namespace vision

// clang-format off
OC_STRUCT(vision, InstanceData, light_id, mat_id, lightmap_id,
          mesh_id, inside_medium, outside_medium, o2w_transposed){
    [[nodiscard]] auto o2w() const noexcept {
        return make_float4x4(transpose(o2w_transposed));
    }
};
// clang-format on

namespace vision {

class UnwrapperResult;

struct MeshHandle {
    uint vertex_buffer;
    uint triangle_buffer;
};

class GPUMesh : public concepts::Noncopyable {
protected:
    RegistrableBuffer<Vertex> vertex_buffer_;
    RegistrableBuffer<Triangle> triangle_buffer_;
    uint index_{InvalidUI32};

public:
    GPUMesh() = default;
    GPUMesh(const vector<Vertex> &vert,
            const vector<Triangle> &triangles) noexcept;
    GPUMesh(uint vert_num, uint tri_num) noexcept;
    GPUMesh(GPUMesh &&other) noexcept;
    GPUMesh &operator=(GPUMesh &&other) noexcept;
    void update_data(uint vert_num, uint tri_num) noexcept;
    void upload_vertices_immediately(const vector<Vertex> &vertices) noexcept;
    void upload_triangles_immediately(const vector<Triangle> &triangles) noexcept;
    [[nodiscard]] BufferUploadCommand *upload_vertices(const vector<Vertex> &vertices) noexcept;
    [[nodiscard]] BufferUploadCommand *upload_triangles(const vector<Triangle> &triangles) noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    OC_MAKE_MEMBER_GETTER_SETTER(vertex_buffer, &)
    OC_MAKE_MEMBER_GETTER_SETTER(triangle_buffer, &)
};

class Mesh : public Hashable, public GPUMesh {
protected:
    vector<Vertex> vertices_;
    vector<Triangle> triangles_;
    uint index_{};

    bool has_lightmap_uv_{false};
    /// light map resolution
    uint2 resolution_;
    /// auto unwrap light map uv is not normalized
    bool normalized_{false};

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;

public:
    Mesh(vector<Vertex> vert, vector<Triangle> tri)
        : vertices_(std::move(vert)), triangles_(std::move(tri)) {
        GPUMesh::update_data(vertices_.size(), triangles_.size());
    }
    Mesh() = default;
    void update_data() noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(vertices, &)
    OC_MAKE_MEMBER_GETTER_SETTER(triangles, &)
    OC_MAKE_MEMBER_GETTER_SETTER(has_lightmap_uv, )
    OC_MAKE_MEMBER_GETTER_SETTER(resolution, )
    void upload_immediately() noexcept;
    [[nodiscard]] CommandList upload() noexcept;
    void normalize_lightmap_uv() noexcept;
    void setup_lightmap_uv(const UnwrapperResult &result);
    [[nodiscard]] float2 lightmap_uv_unnormalized(uint index) const noexcept;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    [[nodiscard]] uint lightmap_size() const noexcept;
    [[nodiscard]] vector<float> surface_areas() const noexcept;
};

}// namespace vision

OC_STRUCT(vision, MeshHandle, vertex_buffer, triangle_buffer){};

#define VS_MAKE_ATTR_SETTER_GETTER(attr)                     \
    void set_##attr(decltype(attr##_.impl()) val) noexcept { \
        if (val.get()) {                                     \
            val->add_reference(shared_from_this());          \
        }                                                    \
        attr##_.init(val);                                   \
    }                                                        \
    void set_##attr##_name(const string &name) noexcept {    \
        attr##_.name = name;                                 \
    }                                                        \
    void set_##attr(decltype(attr##_) val) noexcept {        \
        if (val.get()) {                                     \
            val->add_reference(shared_from_this());          \
        }                                                    \
        attr##_ = ocarina::move(val);                        \
    }                                                        \
    [[nodiscard]] auto attr() const noexcept {               \
        return attr##_.impl();                               \
    }                                                        \
    [[nodiscard]] auto attr() noexcept {                     \
        return attr##_.impl();                               \
    }                                                        \
    [[nodiscard]] bool has_##attr() const noexcept {         \
        return bool(attr());                                 \
    }                                                        \
    [[nodiscard]] string attr##_name() const noexcept {      \
        return attr##_.name;                                 \
    }

namespace vision {
class ShapeInstance : public GUI, public std::enable_shared_from_this<ShapeInstance> {
protected:
    InstanceData handle_;
    float factor_{};
    uint index_{};
    TObject<IAreaLight> emission_{};
    TObject<Material> material_{};
    TObject<Medium> inside_{};
    TObject<Medium> outside_{};
    SP<Mesh> mesh_{};
    string name_;

public:
    Box3f aabb;

public:
    explicit ShapeInstance(SP<Mesh> mesh);
    explicit ShapeInstance(Mesh mesh);
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    OC_MAKE_MEMBER_GETTER_SETTER(mesh, )
    OC_MAKE_MEMBER_GETTER_SETTER(name, )
    OC_MAKE_MEMBER_GETTER_SETTER(handle, &)
    void fill_mesh_id() noexcept;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    void init_aabb() noexcept { aabb = compute_aabb(); }
    VS_MAKE_ATTR_SETTER_GETTER(inside)
    VS_MAKE_ATTR_SETTER_GETTER(outside)
    VS_MAKE_ATTR_SETTER_GETTER(material)
    VS_MAKE_ATTR_SETTER_GETTER(emission)
    void set_lightmap_id(uint id) noexcept { handle_.lightmap_id = id; }
    [[nodiscard]] float4x4 o2w() const noexcept { return handle_.o2w(); }
    void set_o2w(float4x4 o2w) noexcept {
        handle_.o2w_transposed = transpose(make_float4x3(o2w));
    }
    virtual void update_inside_medium_id(uint id) noexcept {
        handle_.inside_medium = id;
    }
    virtual void update_outside_medium_id(uint id) noexcept { handle_.outside_medium = id; }
    virtual void update_material_id(uint id) noexcept { handle_.mat_id = id; }
    virtual void update_light_id(uint id) noexcept { handle_.light_id = id; }
    [[nodiscard]] vector<float> surface_areas() const noexcept;
    [[nodiscard]] bool has_lightmap() const noexcept { return handle_.lightmap_id != InvalidUI32; }
};
}// namespace vision

namespace vision {

class ShapeGroup : public Node, public enable_shared_from_this<ShapeGroup> {
public:
    using Desc = ShapeDesc;

public:
    Box3f aabb;

private:
    vector<SP<ShapeInstance>> instances_;

protected:
    TObject<IAreaLight> emission_{};
    TObject<Material> material_{};

public:
    ShapeGroup() = default;
    explicit ShapeGroup(ShapeInstance inst);
    explicit ShapeGroup(const ShapeDesc &desc);
    [[nodiscard]] string_view impl_type() const noexcept override { return "ShapeGroup"; }
    [[nodiscard]] string_view category() const noexcept override { return "shape"; }
//    VS_MAKE_ATTR_SETTER_GETTER(material)
//    VS_MAKE_ATTR_SETTER_GETTER(emission)
    void post_init(const ShapeDesc &desc);
    [[nodiscard]] ShapeInstance &instance(uint i) noexcept { return *(instances_[i]); }
    [[nodiscard]] const ShapeInstance &instance(uint i) const noexcept { return *(instances_[i]); }
    void add_instance(const ShapeInstance &instance) noexcept;
    void add_instances(vector<ShapeInstance> instances) noexcept;
    void for_each(const std::function<void(SP<const ShapeInstance>, uint)> &func) const noexcept {
        for (uint i = 0; i < instances_.size(); ++i) {
            func(instances_[i], i);
        }
    }
    void for_each(const std::function<void(SP<ShapeInstance>, uint)> &func) noexcept {
        for (uint i = 0; i < instances_.size(); ++i) {
            func(instances_[i], i);
        }
    }
};
#undef VS_MAKE_ATTR_SETTER_GETTER
}// namespace vision