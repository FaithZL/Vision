//
// Created by Zero on 2023/7/6.
//

#pragma once

#include "util.h"
#include "rhi/common.h"
#include "base/mgr/global.h"

namespace vision {

struct BatchMesh : public Context {
private:
    Buffer<Triangle> triangles_;
    Buffer<Vertex> vertices_;
    /**
     * store the pixel data eg.
     * triangle id
     * lightmap offset
     * resolution
     */
    Buffer<uint4> pixels_;
    uint pixel_num_{};
    using signature = void(Buffer<Triangle>, Buffer<Vertex>,
                           Buffer<uint4>, uint, uint, uint2);

    Shader<void(Buffer<uint4>, uint, uint, Buffer<uint4>)> _shader;

public:
    BatchMesh() = default;
    void allocate(uint buffer_size);
    [[nodiscard]] CommandList clear() noexcept;
    void compile() noexcept;
    void batch(ocarina::span<BakedShape> baked_shapes) noexcept;
    OC_MAKE_MEMBER_GETTER(pixel_num, )
    OC_MAKE_MEMBER_GETTER(pixels, &)
    OC_MAKE_MEMBER_GETTER(triangles, &)
    OC_MAKE_MEMBER_GETTER(vertices, &)
};
}// namespace vision