//
// Created by Zero on 2023/6/2.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"
#include "shape.h"
#include "base/mgr/pipeline.h"
#include "mgr/global.h"
#include "descriptions/json_util.h"

namespace vision {

using namespace ocarina;

struct UnwrapperVertex {
    // Not normalized - values are in Atlas width and height range.
    float2 uv{};
    uint xref{};
};

struct UnwrapperMesh {
    vector<UnwrapperVertex> vertices;
    vector<Triangle> triangles;
};

struct UnwrapperResult {
    uint width{};
    uint height{};
    vector<UnwrapperMesh> meshes;
};

struct DeviceMesh {
    Buffer<Vertex> vertices;
    Buffer<Triangle> triangles;
};

struct BakedShape {
private:
    Shape *_shape{};
    uint2 _resolution{};
    RegistrableBuffer<float4> _normals{Global::instance().pipeline()->resource_array()};
    RegistrableBuffer<float4> _positions{Global::instance().pipeline()->resource_array()};
    RegistrableBuffer<float4> _lightmap{Global::instance().pipeline()->resource_array()};
    vector<DeviceMesh> _device_meshes;

public:
    BakedShape() = default;
    explicit BakedShape(Shape *shape) : _shape(shape) {}

    [[nodiscard]] fs::path cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_shape_{:016x}", _shape->hash());
    }

    [[nodiscard]] fs::path instance_cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_instance_{:016x}", instance_hash());
    }

#define VS_MAKE_ATTR_GET(attr, sig) \
    [[nodiscard]] auto sig attr() noexcept { return _##attr; }
    VS_MAKE_ATTR_GET(resolution, )
    VS_MAKE_ATTR_GET(shape, )
    VS_MAKE_ATTR_GET(positions, &)
    VS_MAKE_ATTR_GET(normals, &)
    VS_MAKE_ATTR_GET(lightmap, &)
#undef VS_MAKE_ATTR_GET

    [[nodiscard]] uint64_t instance_hash() const noexcept;
    [[nodiscard]] fs::path uv_config_fn() const noexcept;
    [[nodiscard]] bool has_uv_cache() const noexcept;
    [[nodiscard]] fs::path position_cache_path() const noexcept;
    [[nodiscard]] fs::path normal_cache_path() const noexcept;
    [[nodiscard]] fs::path lightmap_cache_path() const noexcept;
    [[nodiscard]] bool has_rasterization_cache() const noexcept;
    [[nodiscard]] size_t pixel_num() const noexcept { return _resolution.x * _resolution.y; }
    template<typename Func>
    void for_each_device_mesh(const Func &func) noexcept {
        uint i = 0;
        std::for_each(_device_meshes.begin(), _device_meshes.end(), [&](DeviceMesh &mesh) {
            func(mesh, i++);
        });
    }
    void prepare_for_rasterize() noexcept;
    void prepare_for_bake() noexcept;
    [[nodiscard]] UnwrapperResult load_uv_config_from_cache() const;
    void save_to_cache(const UnwrapperResult &result);
    void load_rasterization_from_cache() const;
    void save_rasterization_to_cache() const;
    void save_lightmap_to_cache() const;
    void setup_vertices(UnwrapperResult result);
    void normalize_lightmap_uv();
};

class UVUnwrapper : public Node {
public:
    using Desc = UVUnwrapperDesc;

public:
    explicit UVUnwrapper(const UVUnwrapperDesc &desc)
        : Node(desc) {}
    [[nodiscard]] virtual UnwrapperResult apply(const Shape *shape) = 0;
};

class Rasterizer : public Node {
public:
    using Desc = RasterizerDesc;

public:
    explicit Rasterizer(const RasterizerDesc &desc)
        : Node(desc) {}
    virtual void compile_shader() noexcept = 0;
    virtual void apply(BakedShape &baked_shape) noexcept = 0;
};

}// namespace vision