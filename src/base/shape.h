//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"
#include "dsl/rtx_type.h"
#include "node.h"
#include "math/box.h"
#include "light.h"

namespace vision {

struct Geometry;

struct Shape : public Node {
public:
    using Desc = ShapeDesc;

public:
    struct Handle {
        // todo compress unsigned int data
        uint light_id{InvalidUI32};
        uint mat_id{InvalidUI32};
        uint mesh_id{InvalidUI32};
        uint inside_medium{InvalidUI32};
        uint outside_medium{InvalidUI32};
        float4x4 o2w;
    };

public:
    Box3f aabb;
    Light *emission{};
    Handle handle;

public:
    explicit Shape(const ShapeDesc &desc);
    Shape() = default;
    virtual void fill_geometry(Geometry &data) const noexcept = 0;
    virtual void update_material_id(uint id) noexcept;
    [[nodiscard]] virtual vector<float> surface_area() const noexcept = 0;
};

}// namespace vision

OC_STRUCT(vision::Shape::Handle, light_id, mat_id, mesh_id, inside_medium, outside_medium, o2w){};

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

public:
    explicit Mesh(const ShapeDesc &desc);
    Mesh(vector<Vertex> vert, vector<Triangle> tri)
        : vertices(std::move(vert)), triangles(std::move(tri)) {}
    Mesh() = default;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    void init_aabb() noexcept { aabb = compute_aabb(); }
    void fill_geometry(Geometry &data) const noexcept override;
    [[nodiscard]] vector<float> surface_area() const noexcept override;
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset){};