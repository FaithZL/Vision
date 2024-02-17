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
    vector<Vertex> vertices;
    vector<Triangle> triangles;
};

struct BakedShape : public Context {
private:
    ShapeInstance *_shape{};
    Texture _lightmap_tex;
    MergedMesh _merged_mesh;
    Buffer<uint4> _pixels;

public:
    BakedShape() = default;
    explicit BakedShape(vision::ShapeInstance *shape);

    [[nodiscard]] fs::path cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_shape_{:016x}",
                                                                       _shape->mesh()->hash());
    }

    [[nodiscard]] fs::path instance_cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_instance_{:016x}",
                                                                       instance_hash());
    }

    void prepare_to_rasterize() noexcept;
    void merge_meshes() noexcept;
    [[nodiscard]] uint2 resolution() const noexcept { return _shape->mesh()->resolution(); }
    OC_MAKE_MEMBER_GETTER(shape, )
    OC_MAKE_MEMBER_GETTER(lightmap_tex, &)
    OC_MAKE_MEMBER_GETTER(merged_mesh, &)
    OC_MAKE_MEMBER_GETTER(pixels, &)
    [[nodiscard]] uint64_t instance_hash() const noexcept;
    [[nodiscard]] fs::path uv_config_fn() const noexcept;
    [[nodiscard]] bool has_uv_cache() const noexcept;
    [[nodiscard]] fs::path lightmap_cache_path() const noexcept;
    [[nodiscard]] fs::path rasterize_cache_path() const noexcept;
    void allocate_lightmap_texture() noexcept;
    [[nodiscard]] size_t pixel_num() const noexcept { return resolution().x * resolution().y; }
    [[nodiscard]] uint perimeter() const noexcept { return resolution().x + resolution().y; }
    [[nodiscard]] UnwrapperResult load_uv_config_from_cache() const;
    void save_to_cache(const UnwrapperResult &result);
    [[nodiscard]] CommandList save_lightmap_to_cache() const;
    [[nodiscard]] CommandList save_rasterize_map_to_cache() const;
    void normalize_lightmap_uv();
};

class UVUnwrapper : public Node {
public:
    using Desc = UVUnwrapperDesc;

protected:
    uint _padding{};
    float _scale{};
    uint _min{};
    uint _max{};

public:
    explicit UVUnwrapper(const UVUnwrapperDesc &desc)
        : Node(desc), _padding(desc["padding"].as_uint(3)),
          _scale(desc["scale"].as_float(1.f)),
          _min(desc["min"].as_uint(50)),
          _max(desc["max"].as_uint(1024)) {}
    [[nodiscard]] virtual UnwrapperResult apply(const Mesh *shape) = 0;
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