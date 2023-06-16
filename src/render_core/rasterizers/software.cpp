//
// Created by Zero on 2023/6/15.
//

#include "base/bake_utlis.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
private:
    using signature = void(Buffer<Vertex>, Buffer<Triangle>, Buffer<float4>, Buffer<float4>);
    Shader<signature> _vertex_shader;

public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile_shader() noexcept override {
        Kernel vertex_kernel = [&](BufferVar<Vertex> vertices, BufferVar<Triangle> triangles,
                                   BufferVar<float4> position, BufferVar<float4> normal) {

        };
        _vertex_shader = device().compile(vertex_kernel);
    }

    void apply(vision::BakedShape &baked_shape) noexcept override {
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)