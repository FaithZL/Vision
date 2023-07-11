//
// Created by Zero on 2023/7/10.
//

#include "base/bake_utlis.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
private:
    using signature = void(Buffer<Vertex>, Buffer<Triangle>, uint,
                           Buffer<uint4>, Buffer<float4>);
    Shader<signature> _shader;

public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile() noexcept override {
        Sampler *sampler = scene().sampler();
        Kernel kernel = [&](BufferVar<Vertex> vertices, BufferVar<Triangle> triangles,
                            Uint triangle_index, BufferVar<uint4> pixels,
                            BufferVar<float4> debug_buffer) {
            Float2 coord = (make_float2(dispatch_idx().xy()) + 0.5f);
            Var tri = triangles.read(triangle_index);
            Var v0 = vertices.read(tri.i);
            Var v1 = vertices.read(tri.j);
            Var v2 = vertices.read(tri.k);

            Float2 p0 = v0->lightmap_uv();
            Float2 p1 = v1->lightmap_uv();
            Float2 p2 = v2->lightmap_uv();
            Uint4 pixel = pixels.read(dispatch_id());
            $if(in_triangle<D>(coord, p0, p1, p2)) {
                sampler->start_pixel_sample(make_uint2(triangle_index, 0u), 0u, 0u);
                pixel.x = triangle_index;
                Float2 u = sampler->next_2d();
                pixel.y = as<uint>(u.x);
                pixel.z = as<uint>(u.y);
                pixel.w = as<uint>(1.f);
                pixels.write(dispatch_id(), pixel);
                debug_buffer.write(dispatch_id(), make_float4(u, sampler->next_1d(), 1.f));
            };
        };
        _shader = device().compile(kernel, "rasterizer");
    }

    void apply(vision::BakedShape &bs) noexcept override {
        auto &stream = pipeline()->stream();
        MergedMesh &mesh = bs.merged_mesh();
        for (uint i = 0; i < mesh.triangles.host_buffer().size(); ++i) {
            stream << _shader(mesh.vertices, mesh.triangles, i,
                              bs.pixels(), bs.debug_pixels())
                          .dispatch(bs.resolution());
        }
        stream << synchronize() << commit();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)