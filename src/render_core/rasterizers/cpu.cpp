//
// Created by Zero on 2023/7/11.
//


#include "base/bake_utlis.h"

namespace vision {

class GPURasterizer : public Rasterizer {
private:

public:
    explicit GPURasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile() noexcept override {}

    void draw(Vertex v0, Vertex v1, Vertex v2, uint4 *pixel,
              uint2 res, uint index) const noexcept {

        ocarina::array<Vertex, 3> arr = {v0, v1, v2};
        std::sort(arr.begin(), arr.end(), [](const Vertex &a, const Vertex &b) {
            return a.lightmap_uv().y > b.lightmap_uv().y;
        });
        v0 = arr[0];
        v1 = arr[1];
        v2 = arr[2];

        auto equal = [](float a, float b) {
            return ocarina::round(a) == ocarina::round(b);
        };

        auto draw_up = [pixel, res, index](float2 p0, float2 p1, float2 p2) {
            uint start_y = ocarina::round(p0.y);
            uint end_y = ocarina::round(p2.y);
        };

        auto draw_down = [pixel, res, index](float2 p0, float2 p1, float2 p2) {
            uint start_y = ocarina::round(p0.y);
            uint end_y = ocarina::round(p2.y);
        };

        float2 p0 = v0.lightmap_uv();
        float2 p1 = v1.lightmap_uv();
        float2 p2 = v2.lightmap_uv();

        if (equal(p0.y, p1.y)) {
            draw_down(p0, p1, p2);
        } else if (equal(p0.y, p2.y)) {
            draw_up(p0, p1, p2);
        }

    }

    void apply(vision::BakedShape &bs) noexcept override {
        const MergedMesh &mesh = bs.merged_mesh();
        auto &stream = pipeline()->stream();
        vector<uint4> pixels;
        pixels.resize(bs.pixel_num());
        const vector<Vertex> &vertices = mesh.vertices;
        const vector<Triangle> &triangles = mesh.triangles;
        for (uint i = 0; i < triangles.size(); ++i) {
            Triangle tri = triangles.at(i);
            Vertex v0 = vertices.at(tri.i);
            Vertex v1 = vertices.at(tri.j);
            Vertex v2 = vertices.at(tri.k);
            draw(v0, v1, v2, pixels.data(), bs.resolution(), i);
        }
        bs.pixels().upload_immediately(pixels.data());
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GPURasterizer)