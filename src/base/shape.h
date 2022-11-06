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
    Box3f aabb;
    float4x4 o2w;
    uint light_id{InvalidUI32};
    uint mat_id{InvalidUI32};
    struct Handle {
        uint light_id{InvalidUI32};
        uint mat_id{InvalidUI32};
        uint mesh_id{InvalidUI32};
        float4x4 o2w;
    };

public:
    explicit Shape(const ShapeDesc &desc);
    Shape() = default;
    virtual void fill_geometry(Geometry &data) const noexcept = 0;
    [[nodiscard]] virtual vector<float> surface_area() const noexcept = 0;
};

}// namespace vision

OC_STRUCT(vision::Shape::Handle, light_id, mat_id, mesh_id, o2w){};

namespace vision {
struct Mesh : public Shape {
public:
    struct Handle {
        uint vertex_offset;
        uint triangle_offset;
    };

public:
    Light *emission{};
    vector<Vertex> vertices;
    vector<Triangle> triangles;

public:
    explicit Mesh(const ShapeDesc &desc);
    Mesh() = default;
    void fill_geometry(Geometry &data) const noexcept override;
    [[nodiscard]] vector<float> surface_area() const noexcept override;
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset){};