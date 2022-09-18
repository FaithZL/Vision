//
// Created by Zero on 19/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/struct.h"

namespace vision {
inline namespace geometry {
struct Triangle {
    uint i, j, k;
};
}
}// namespace vision::geometry
OC_STRUCT(vision::Triangle, i, j, k){};

namespace vision {
inline namespace geometry {
struct Frame {
    float3 x, y, z;
};
}
}// namespace vision::geometry
OC_STRUCT(vision::Frame, x, y, z){};

namespace vision {
inline namespace geometry {
struct Vertex {
    //todo compress
    float3 position;
    float3 normal;
    float3 tex_coord;
};
}
}// namespace vision::geometry
OC_STRUCT(vision::Vertex, position, normal, tex_coord){};

namespace vision {
inline namespace geometry {
struct Mesh {
    uint vertex_offset;
    uint triangle_offset;
    uint vertex_count;
    uint triangle_count;
    uint distribute_idx;
    uint material_idx;
    uint light_idx;
};
}
}// namespace vision::geometry
OC_STRUCT(vision::Mesh, vertex_offset, triangle_offset, vertex_count,
          triangle_count, distribute_idx, material_idx, light_idx){};