//
// Created by Zero on 2023/7/6.
//

#pragma once

#include "base/bake_utlis.h"
#include "rhi/common.h"
#include "base/mgr/global.h"

namespace vision {

struct BatchMesh : public Ctx {
private:
    Managed<Triangle> _triangles;
    Managed<Vertex> _vertices;
    /**
     * store the pixel data eg.
     * triangle id
     * lightmap offset
     * resolution
     */
    Buffer<uint4> _pixels;
    vector<uint> _triangle_nums;
    vector<uint2> _resolutions;
    using signature = void(Buffer<Triangle>, Buffer<Vertex>,
                           Buffer<uint4>, uint, uint, uint2);
    Shader<signature> _rasterizer;
private:
    void append(const BakedShape &bs) noexcept;

public:
    BatchMesh() = default;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] CommandList rasterize() const noexcept;
    void compile() noexcept;
    void setup(ocarina::span<BakedShape> baked_shapes, uint buffer_size) noexcept;
    OC_MAKE_MEMBER_GETTER(pixels, &)
    OC_MAKE_MEMBER_GETTER(triangles, &)
    OC_MAKE_MEMBER_GETTER(vertices, &)
};
}// namespace vision