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
    using Pixel = uint4;
    Managed<Triangle> _triangle;
    Managed<Vertex> _vertices;
    /**
     * store the pixel data eg.
     * triangle id
     * lightmap offset
     * resolution
     */
    Buffer<Pixel> _pixels;
    vector<uint> _triangle_nums;
    vector<uint> _pixel_nums;
    using signature = void(Buffer<Triangle>, Buffer<Vertex>,
                           Buffer<Pixel>, uint, uint, uint2);
    Shader<signature> _rasterizer;

public:
    BatchMesh() = default;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] CommandList rasterize() const noexcept;
    void compile() noexcept;
    void append(const BakedShape &bs) noexcept;
    OC_MAKE_MEMBER_GETTER(pixels, &)
    OC_MAKE_MEMBER_GETTER(triangle, &)
    OC_MAKE_MEMBER_GETTER(vertices, &)
};
}// namespace vision