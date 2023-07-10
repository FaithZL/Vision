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

struct BakedShape {
private:
    Shape *_shape{};
    uint2 _resolution{};
    Texture _lightmap_tex;

public:
    BakedShape() = default;
    explicit BakedShape(Shape *shape) : _shape(shape) {}

    [[nodiscard]] fs::path cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_shape_{}_{:016x}",
                                                                       _shape->name(),
                                                                       _shape->hash());
    }

    [[nodiscard]] fs::path instance_cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_instance_{}_{:016x}",
                                                                       _shape->name(),
                                                                       instance_hash());
    }

    OC_MAKE_MEMBER_GETTER(resolution, )
    OC_MAKE_MEMBER_GETTER(shape, )
    OC_MAKE_MEMBER_GETTER(lightmap_tex, &)
    [[nodiscard]] uint64_t instance_hash() const noexcept;
    [[nodiscard]] fs::path uv_config_fn() const noexcept;
    [[nodiscard]] bool has_uv_cache() const noexcept;
    [[nodiscard]] fs::path lightmap_cache_path() const noexcept;
    void allocate_lightmap_texture() noexcept;
    [[nodiscard]] size_t pixel_num() const noexcept { return _resolution.x * _resolution.y; }
    [[nodiscard]] uint perimeter() const noexcept { return resolution().x + resolution().y; }
    [[nodiscard]] UnwrapperResult load_uv_config_from_cache() const;
    void save_to_cache(const UnwrapperResult &result);
    [[nodiscard]] CommandList save_lightmap_to_cache() const;
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

}// namespace vision