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

struct Shape : public Node {
public:
    using Desc = ShapeDesc;

public:
    Box3f aabb;
    float4x4 o2w;

public:
    explicit Shape(const ShapeDesc &desc);
    [[nodiscard]] virtual size_t sub_shape_num() const noexcept { return 1; }
};

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
    uint inst_id{InvalidUI32};
    vector<Vertex> vertices;
    vector<Triangle> triangles;

public:
    explicit Mesh(const ShapeDesc &desc);
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset, vertex_count,
          triangle_count){};