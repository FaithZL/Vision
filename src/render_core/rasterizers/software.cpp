//
// Created by Zero on 2023/6/15.
//

#include "base/bake.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
private:
    using signature = void(Buffer<Vertex>, Buffer<Triangle>, Buffer<float2>);
    Shader<signature> _shader;

public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile_shader() noexcept override {
        Kernel kernel = [&](BufferVar<Vertex> vertices,
                            BufferVar<Triangle> triangles,
                            BufferVar<float2> uv2) {
            Var v = vertices.read(0);
            Var t = triangles.read(0);
        };
        _shader = device().compile(kernel);
    }

    void apply(vision::BakedShape &baked_shape) noexcept override {
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)