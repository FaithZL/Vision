//
// Created by Zero on 2023/7/11.
//

#include "base/bake_utlis.h"

namespace vision {

class GPURasterizer : public Rasterizer {
private:
    uint4 *_pixels{};
    uint2 _res{};

public:
    explicit GPURasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile() noexcept override {}

    void write(uint x, uint y, uint4 val) noexcept {
        OC_ASSERT(all(make_uint2(x,y) < _res));
        uint index = y * _res.x + x;
        _pixels[index] = val;
    }

    void scan_line(float2 p0, float2 p1, uint tri_index) noexcept {
        if (p0.x > p1.x) {
            std::swap(p0, p1);
        }
        float d = p1.x - p0.x;
        uint y = p0.y;
        for (uint i = 0; i < d; ++i) {
            uint x = p0.x + i;
            uint4 val = make_uint4(tri_index,as<uint>(1.f),as<uint>(1.f),as<uint>(1.f));
            write(x, y, val);
        }
    }

    void draw(Vertex v0, Vertex v1, Vertex v2, uint index) noexcept {

        ocarina::array<Vertex, 3> arr = {v0, v1, v2};
        std::sort(arr.begin(), arr.end(), [](const Vertex &a, const Vertex &b) {
            return a.lightmap_uv().y > b.lightmap_uv().y;
        });
        v0 = arr[0];
        v1 = arr[1];
        v2 = arr[2];

        auto equal = [](float a, float b) {
            return uint(a) == uint(b);
        };

        auto draw_top = [this, index](float2 p0, float2 p1, float2 p2) {
            uint start_y = p2.y;
            uint end_y = p0.y;

            for (uint py = start_y; py < end_y; ++py) {
                float factor = float(py - start_y) / (end_y - start_y);
                float2 p_start = lerp(factor, p1, p0);
                float2 p_end = lerp(factor, p2, p0);
                scan_line(p_start, p_end, index);
            }
        };

        auto draw_bottom = [this, index](float2 p0, float2 p1, float2 p2) {
            uint start_y = p2.y;
            uint end_y = p0.y;

            for (uint py = start_y; py < end_y; py += 1) {
                float factor = float(py - start_y) / (end_y - start_y);
                float2 p_start = lerp(factor, p2, p0);
                float2 p_end = lerp(factor, p2, p1);
                scan_line(p_start, p_end, index);
            }
        };

        float2 p0 = v0.lightmap_uv();
        float2 p1 = v1.lightmap_uv();
        float2 p2 = v2.lightmap_uv();

        if (equal(p0.y, p1.y)) {
            draw_bottom(p0, p1, p2);
        } else if (equal(p1.y, p2.y)) {
            draw_top(p0, p1, p2);
        } else {
            float factor = float(p1.y - p2.y) / (p0.y - p2.y);
            float2 pc = lerp(factor, p2, p0);
            draw_top(p0, pc, p1);
            draw_bottom(pc, p1, p2);
        }
    }

    void apply(vision::BakedShape &bs) noexcept override {
        const MergedMesh &mesh = bs.merged_mesh();
        auto &stream = pipeline()->stream();
        vector<uint4> pixels;
        pixels.resize(bs.pixel_num());
        _pixels = pixels.data();
        _res = bs.resolution();
        const vector<Vertex> &vertices = mesh.vertices;
        const vector<Triangle> &triangles = mesh.triangles;
        for (uint i = 0; i < triangles.size(); ++i) {
            Triangle tri = triangles.at(i);
            Vertex v0 = vertices.at(tri.i);
            Vertex v1 = vertices.at(tri.j);
            Vertex v2 = vertices.at(tri.k);
            draw(v0, v1, v2, i);
        }
        bs.pixels().upload_immediately(_pixels);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::GPURasterizer)