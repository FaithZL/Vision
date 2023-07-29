//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"
#include "dsl/rtx_type.h"
#include "node.h"
#include "math/box.h"
#include "core/vs_header.h"
#include "base/scattering/material.h"
#include "base/illumination/light.h"

namespace vision {

struct Geometry;
struct Mesh;

struct Shape : public Node {
public:
    using Desc = ShapeDesc;

public:
    struct Handle {
        // todo compress unsigned int data
        uint light_id{InvalidUI32};
        uint mat_id{InvalidUI32};
        uint lightmap_id{InvalidUI32};
        uint mesh_id{InvalidUI32};
        uint inside_medium{InvalidUI32};
        uint outside_medium{InvalidUI32};
        float4x4 o2w;
    };

public:
    Box3f aabb;
    SP<Light> emission{};
    SP<Material> material{};
    std::string mat_name;

protected:
    Handle _handle;
    float _factor{};

public:
    explicit Shape(const ShapeDesc &desc);
    Shape() = default;
    virtual void fill_geometry(Geometry &data) const noexcept {
        OC_ASSERT(false);
        OC_ERROR("fill_geometry can not called by model");
    }
    void load_light(const LightDesc &desc) noexcept;
    virtual void update_material_id(uint id) noexcept { _handle.mat_id = id; }
    virtual void update_light_id(uint id) noexcept { _handle.light_id = id; }
    [[nodiscard]] bool has_material() const noexcept { return material.get(); }
    [[nodiscard]] bool has_lightmap() const noexcept { return _handle.lightmap_id != InvalidUI32; }
    [[nodiscard]] virtual vector<float> ref_surface_areas() const noexcept {
        OC_ASSERT(false);
        OC_ERROR("ref_surface_areas can not called by model");
    }
    [[nodiscard]] virtual uint lightmap_size() const noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(handle, &)
    virtual void set_lightmap_id(uint id) noexcept {
        OC_ASSERT(false);
        OC_ERROR("set_lightmap_id can not called by model");
    }
    [[nodiscard]] bool has_emission() const noexcept { return emission.get(); }
    [[nodiscard]] virtual vector<float> surface_areas() const noexcept {
        OC_ASSERT(false);
        OC_ERROR("surface_areas can not called by model");
    }
    virtual void for_each_mesh(const std::function<void(vision::Mesh &, uint)> &func) noexcept = 0;
    virtual void for_each_mesh(const std::function<void(const vision::Mesh &, uint)> &func) const noexcept = 0;
    [[nodiscard]] virtual Mesh &mesh_at(uint i) noexcept = 0;
    [[nodiscard]] virtual const Mesh &mesh_at(uint i) const noexcept = 0;
    [[nodiscard]] virtual float4x4 o2w() const noexcept { return _handle.o2w; }
};

}// namespace vision

OC_STRUCT(vision::Shape::Handle, light_id, mat_id, lightmap_id, mesh_id, inside_medium, outside_medium, o2w){};

namespace vision {
struct Mesh : public Shape {
public:
    struct Handle {
        uint vertex_offset;
        uint triangle_offset;
    };

public:
    vector<Vertex> vertices;
    vector<Triangle> triangles;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;

public:
    explicit Mesh(const ShapeDesc &desc);
    Mesh(vector<Vertex> vert, vector<Triangle> tri)
        : vertices(std::move(vert)), triangles(std::move(tri)) {}
    Mesh() = default;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    void init_aabb() noexcept { aabb = compute_aabb(); }
    void fill_geometry(Geometry &data) const noexcept override;
    [[nodiscard]] vector<float> surface_areas() const noexcept override;
    [[nodiscard]] vector<float> ref_surface_areas() const noexcept override;
    void set_lightmap_id(ocarina::uint id) noexcept override { handle().lightmap_id = id; }
    void for_each_mesh(const std::function<void(vision::Mesh &, uint)> &func) noexcept override {
        func(*this, 0);
    }
    void for_each_mesh(const std::function<void(const vision::Mesh &, uint)> &func) const noexcept override {
        func(*this, 0);
    }
    [[nodiscard]] const Mesh &mesh_at(ocarina::uint i) const noexcept override {
        return *this;
    }
    [[nodiscard]] Mesh &mesh_at(ocarina::uint i) noexcept override {
        return *this;
    }
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset){};