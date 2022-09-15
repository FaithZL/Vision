//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"
#include "dsl/rtx_type.h"

namespace vision {

struct Shape {};

struct Mesh : public Shape {
    vector<float3> normals;
    vector<float3> positions;
    vector<float2> tex_coords;
    vector<Triangle> triangles;
    uint mat_idx{InvalidUI32};
};

}// namespace vision