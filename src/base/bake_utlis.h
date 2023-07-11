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
    uint chart_idx{};
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

struct MergedMesh {
    Managed<Vertex> vertices;
    Managed<Triangle> triangles;

    void upload(Device &device) noexcept {
        triangles.reset_device_buffer_immediately(device);
        vertices.reset_device_buffer_immediately(device);
        triangles.upload_immediately();
        vertices.upload_immediately();
    }
};

struct BakedShape {
private:
    Shape *_shape{};
    uint2 _resolution{};
    Texture _lightmap_tex;
    MergedMesh _merged_mesh;
    Buffer<uint4> _pixels;
    Buffer<float4> _debug_pixels;

public:
    BakedShape() = default;
    explicit BakedShape(Shape *shape) : _shape(shape) {}

    [[nodiscard]] fs::path cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_shape_{:016x}",
                                                                       _shape->hash());
    }

    [[nodiscard]] fs::path instance_cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_instance_{}_{:016x}",
                                                                       _shape->name(),
                                                                       instance_hash());
    }

    void prepare_to_bake() noexcept;
    void prepare_to_rasterize() noexcept;
    void merge_meshes() noexcept;

    OC_MAKE_MEMBER_GETTER(resolution, )
    OC_MAKE_MEMBER_GETTER(shape, )
    OC_MAKE_MEMBER_GETTER(lightmap_tex, &)
    OC_MAKE_MEMBER_GETTER(merged_mesh, &)
    OC_MAKE_MEMBER_GETTER(pixels, &)
    OC_MAKE_MEMBER_GETTER(debug_pixels, &)
    [[nodiscard]] uint64_t instance_hash() const noexcept;
    [[nodiscard]] fs::path uv_config_fn() const noexcept;
    [[nodiscard]] bool has_uv_cache() const noexcept;
    [[nodiscard]] fs::path lightmap_cache_path() const noexcept;
    [[nodiscard]] fs::path rasterize_cache_path() const noexcept;
    [[nodiscard]] fs::path rasterize_debug_path() const noexcept;
    void allocate_lightmap_texture() noexcept;
    [[nodiscard]] size_t pixel_num() const noexcept { return _resolution.x * _resolution.y; }
    [[nodiscard]] uint perimeter() const noexcept { return resolution().x + resolution().y; }
    [[nodiscard]] UnwrapperResult load_uv_config_from_cache() const;
    void save_to_cache(const UnwrapperResult &result);
    [[nodiscard]] CommandList save_lightmap_to_cache() const;
    [[nodiscard]] CommandList save_rasterize_map_to_cache() const;
    void setup_vertices(UnwrapperResult result);
    void normalize_lightmap_uv();
};

class UVUnwrapper : public Node {
public:
    using Desc = UVUnwrapperDesc;

protected:
    uint _padding{};

public:
    explicit UVUnwrapper(const UVUnwrapperDesc &desc)
        : Node(desc), _padding(desc["padding"].as_uint(3)) {}
    [[nodiscard]] virtual UnwrapperResult apply(const Shape *shape) = 0;
};

class Rasterizer : public Node {
public:
    using Desc = RasterizerDesc;

public:
    explicit Rasterizer(const RasterizerDesc &desc)
        : Node(desc) {}
    virtual void compile() noexcept = 0;
    virtual void apply(BakedShape &baked_shape) noexcept = 0;
};

}// namespace vision