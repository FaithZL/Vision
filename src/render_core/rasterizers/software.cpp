//
// Created by Zero on 2023/6/15.
//

#include "base/bake_utlis.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
private:
    using signature = void(Buffer<Vertex>, Buffer<Triangle>, Buffer<float4>, Buffer<float4>, uint2, uint);
    Shader<signature> _shader;

public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile_shader() noexcept override {
        Kernel kernel = [&](BufferVar<Vertex> vertices, BufferVar<Triangle> triangles,
                            BufferVar<float4> positions, BufferVar<float4> normals, Uint2 res, Uint triangle_index) {
            Float2 coord = (make_float2(dispatch_idx().xy()) + 0.5f);
            Var tri = triangles.read(triangle_index);
            Var v0 = vertices.read(tri.i);
            Var v1 = vertices.read(tri.j);
            Var v2 = vertices.read(tri.k);

            Float2 p0 = v0->lightmap_uv();
            Float2 p1 = v1->lightmap_uv();
            Float2 p2 = v2->lightmap_uv();
            $if(in_triangle<D>(coord, p0, p1, p2)) {
                Float2 bary = barycentric(coord, p0, p1, p2);
                Float3 pos = triangle_lerp(bary, v0->position(), v1->position(), v2->position());
                positions.write(dispatch_id(), make_float4(pos, 1.f));

                Float3 n0 = v0->normal();
                Float3 n1 = v1->normal();
                Float3 n2 = v2->normal();
                Float3 norm;
                $if(is_zero(n0) || is_zero(n1) || is_zero(n2)) {
                    Var v02 = v2->position() - v0->position();
                    Var v01 = v1->position() - v0->position();
                    norm = normalize(cross(v01, v02));
                } $else {
                    norm = normalize(triangle_lerp(bary, n0, n1, n2));
                };
                normals.write(dispatch_id(), make_float4(norm, 1.f));
            };
        };
        _shader = device().compile(kernel, "rasterizer");
    }

    [[nodiscard]] CommandList apply(vision::BakedShape &baked_shape) noexcept override {
        CommandList ret;

        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)