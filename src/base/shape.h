//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"
#include "dsl/rtx_type.h"
#include "node.h"

namespace vision {

struct Shape : public Node {
    using Desc = ShapeDesc;
};

struct Mesh : public Shape {
public:
    struct Handle {
        uint vertex_offset;
        uint triangle_offset;
        uint vertex_count;
        uint triangle_count;
        uint distribute_idx;
        uint material_idx;
        uint light_idx;
    };

public:
    vector<Vertex> vertices;
    vector<Triangle> triangles;
    uint mat_idx{InvalidUI32};
    uint light_idx{InvalidUI32};
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset, vertex_count,
          triangle_count, distribute_idx, material_idx, light_idx){};