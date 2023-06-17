//
// Created by Zero on 2023/6/15.
//

#include "base/bake_utlis.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
private:
    using signature = void(Buffer<Vertex>, Buffer<Triangle>, Buffer<float4>, Buffer<float4>, uint2);
    Shader<signature> _vertex_shader;

public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile_shader() noexcept override {
        Kernel vertex_kernel = [&](BufferVar<Vertex> vertices, BufferVar<Triangle> triangles,
                                   BufferVar<float4> position, BufferVar<float4> normal, Uint2 res) {
//            Var triangle = triangles.read(dispatch_id());
            Var vertex = vertices.read(dispatch_id());

            Printer::instance().info("wocao {} {} {} {}", vertex->lightmap_uv(), res);
        };
        _vertex_shader = device().compile(vertex_kernel);
    }

    void apply(vision::BakedShape &baked_shape) noexcept override {
        auto &stream = pipeline()->stream();
        baked_shape.for_each_device_mesh([&](DeviceMesh &device_mesh) {
            stream << _vertex_shader(device_mesh.vertices, device_mesh.triangles,
                                     baked_shape.position(),
                                     baked_shape.normal(),
                                     make_uint2(baked_shape.resolution())).dispatch(device_mesh.triangles.size());
        });
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)