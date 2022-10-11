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
    vector<Vertex> vertices;
    vector<Triangle> triangles;
    uint mat_idx{InvalidUI32};
    uint light_idx{InvalidUI32};
};

}// namespace vision