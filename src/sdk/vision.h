//
// Created by Zero on 2023/7/17.
//

#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <array>
#include <vector>

namespace vision {

namespace sdk {

using std::array;
using std::vector;

struct Vertex {
public:
    array<float, 3> pos;
    array<float, 3> n;
    array<float, 2> uv;
    array<float, 2> uv2;
};

struct Triple {
public:
    uint32_t i, j, k;
};

// clang-format off
struct Mat4x4 {
public:
    array<float, 16> m;
    static Mat4x4 identity() noexcept {
        return {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        };
    }
};
// clang-format on

struct Instance {
    vector<Vertex> vertices;
    vector<Triple> triangles;
    uint32_t mat_id{~0u};
    uint32_t light_id{~0u};
    Mat4x4 mat4{Mat4x4::identity()};
};

class VisionRenderer {
public:
    virtual ~VisionRenderer() = default;
    virtual void init_device() = 0;
    virtual void add_instance(Instance instance) = 0;
    //    virtual void add_light() = 0;
    //    virtual void add_material() = 0;
    virtual void build_accel() = 0;
};
}

}// namespace vision::sdk

using visionCreator = vision::sdk::VisionRenderer *();
using visionDeleter = void(vision::sdk::VisionRenderer *);