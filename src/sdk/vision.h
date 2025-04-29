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
#include <memory>
#include <windows.h>
#include "json.hpp"

namespace vision::sdk {

using Value = nlohmann::json;

using std::array;
using std::vector;

struct Vertex {
public:
    array<float, 3> pos{};
    array<float, 3> n{};
    array<float, 2> uv{};
    array<float, 2> uv2{};
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
    std::shared_ptr<Triple> triangles{};
    uint32_t tri_num{~0u};
    std::shared_ptr<Vertex> vertices{};
    uint32_t vert_num{~0u};
    Mat4x4 mat4{Mat4x4::identity()};
    std::string mat{};
};

struct Sensor {
    Mat4x4 c2w{Mat4x4::identity()};
    float fov_y{35};
};

class VisionRenderer {
public:
    virtual ~VisionRenderer() = default;
    virtual void init_pipeline(const char *rpath) = 0;
    virtual void init_scene() = 0;
    virtual void compile() = 0;
    virtual void render() = 0;
    virtual void invalidation() = 0;
    virtual void clear_geometries() = 0;
    virtual void add_instance(const Instance &instance, const char *mat) = 0;
    virtual void build_accel() = 0;
    virtual void update_camera(Sensor camera) = 0;
    virtual void update_resolution(uint32_t width, uint32_t height) = 0;
    virtual void download_radiance(void *data) = 0;
};
}// namespace vision::sdk

using visionCreator = vision::sdk::VisionRenderer *();
using visionDeleter = void(vision::sdk::VisionRenderer *);

template<typename T>
T *find_symbol(HMODULE &handle, const char *name_view) noexcept {
    auto symbol = GetProcAddress(handle, name_view);
    return reinterpret_cast<T *>(symbol);
}

static visionCreator *vision_creator(HMODULE &module) {
    module = LoadLibraryA("vision-renderer");
    return find_symbol<visionCreator>(module, "create");
}

static vision::sdk::VisionRenderer *create_vision(HMODULE &module) {
    return vision_creator(module)();
}