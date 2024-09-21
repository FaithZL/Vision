//
// Created by Zero on 2024/9/21.
//

#pragma once

#include "core/stl.h"
#include "math/basic_types.h"
#include "base/node.h"
#include "dsl/dsl.h"

namespace vision {
struct LineSegment {
    float3 start;
    float3 end;
};
}// namespace vision

//clang-format off
OC_STRUCT(vision, LineSegment, start, end){};
//clang-format on

namespace vision {

using namespace ocarina;

class Visualizer {
private:
    RegistrableManaged<LineSegment> line_segments_;

public:
    void draw(const float4 *data, uint2 res) const noexcept;
};

}// namespace vision