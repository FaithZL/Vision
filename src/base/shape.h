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

class RenderData;

struct Shape : public Node {
public:
    using Desc = ShapeDesc;

public:
    Box3f aabb;
    float4x4 o2w;

    struct Handle {
        uint mat_id;
        uint light_id;
        uint mesh_id;
    };

public:
    explicit Shape(const ShapeDesc &desc);
    Shape() = default;
    virtual void fill_render_data(RenderData &data, size_t *inst_id) const noexcept {}
};

}// namespace vision

OC_STRUCT(vision::Shape::Handle, mat_id, light_id, mesh_id) {};

namespace vision {
struct Mesh : public Shape {
public:
    struct Handle {
        uint vertex_offset;
        uint triangle_offset;
        uint vertex_count;
        uint triangle_count;
    };

public:
    uint mat_idx{InvalidUI32};
    Light *emission{};
    vector<Vertex> vertices;
    vector<Triangle> triangles;

public:
    explicit Mesh(const ShapeDesc &desc);
    Mesh() = default;
    void fill_render_data(RenderData &data, size_t *inst_id) const noexcept {}
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset, vertex_count,
          triangle_count){};