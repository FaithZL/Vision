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
#include <windows.h>

namespace vision::sdk {

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

struct Camera {
    Mat4x4 c2w{Mat4x4::identity()};
    float fov_y{35};
};

class VisionRenderer {
public:
    virtual ~VisionRenderer() = default;
    virtual void init_pipeline() = 0;
    virtual void init_scene() = 0;
    virtual void add_instance(Instance instance) = 0;
    virtual void build_accel() = 0;
    virtual void update_camera(Camera camera) = 0;
    virtual void update_resolution(uint32_t width, uint32_t height) = 0;
};
}// namespace vision::sdk

using visionCreator = vision::sdk::VisionRenderer *();
using visionDeleter = void(vision::sdk::VisionRenderer *);