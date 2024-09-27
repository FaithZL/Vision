//
// Created by Zero on 2023/7/11.
//

#include "base/bake_utlis.h"
#include "math/util.h"

namespace vision {

class CPURasterizer : public RasterizerImpl {
private:
    uint4 *pixels_{};
    uint2 res_{};
    static constexpr ocarina::array<float2, 10> _color{make_float2(0.5, 0),
                                                       make_float2(0, 1),
                                                       make_float2(0, 0.5),
                                                       make_float2(0.3, 1),
                                                       make_float2(1, 0.3),
                                                       make_float2(0.3, 0.6),
                                                       make_float2(0.6, 0.3),
                                                       make_float2(0.25, 0.75),
                                                       make_float2(0.75, 0.25),
                                                       make_float2(1, 1)};

public:
    explicit CPURasterizer(const RasterizerDesc &desc)
        : RasterizerImpl(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    void compile() noexcept override {}

    void write(uint x, uint y, uint4 val) noexcept {
        OC_ASSERT(all(make_uint2(x, y) <= res_));
        x = ocarina::clamp(x, 0u, res_.x - 1u);
        y = ocarina::clamp(y, 0u, res_.y - 1u);
        uint index = y * res_.x + x;
        pixels_[index] = val;
    }

    void scan_line(float2 p0, float2 p1, uint tri_index) noexcept {
        if (p0.x > p1.x) {
            std::swap(p0, p1);
        }
        uint y = p0.y;
        for (uint i = p0.x; i <= (p1.x); ++i) {
            uint x = i;
            uint4 val = make_uint4(tri_index,
                                   as<uint>(1.f),
                                   as<uint>(1.f),
                                   as<uint>(1.f));
            write(x, y, val);
        }
    }

    void draw_top(float2 p0, float2 p1, float2 p2, uint tri_idx) {
        uint start_y = p2.y;
        uint end_y = p0.y;
        float factor;
        float2 p_start;
        float2 p_end;
        for (uint py = start_y; py <= end_y; ++py) {
            factor = float(py - start_y) / (end_y - start_y);
            factor = ocarina::clamp(factor, 0.f, 1.f);
            p_start = lerp(factor, p1, p0);
            p_end = lerp(factor, p2, p0);
            scan_line(p_start, p_end, tri_idx);
        }
    };

    void draw_bottom(float2 p0, float2 p1, float2 p2, uint tri_idx) {
        uint start_y = p2.y;
        uint end_y = p0.y;
        float factor;
        for (uint py = start_y; py <= end_y; py += 1) {
            factor = float(py - start_y) / (end_y - start_y);
            factor = ocarina::clamp(factor, 0.f, 1.f);
            float2 p_start = lerp(factor, p2, p0);
            float2 p_end = lerp(factor, p2, p1);
            scan_line(p_start, p_end, tri_idx);
        }
    };

    void draw_bound(float2 p0, float2 p1, float2 p2, uint tri_idx) noexcept {
        Box2u box;
        box.extend(make_uint2(p0));
        box.extend(make_uint2(p1));
        box.extend(make_uint2(p2));
        box.upper += make_uint2(1);

        uint color_idx = tri_idx % _color.size();

        float2 color = _color[color_idx];

        box.for_each([&](uint2 up) {
            float2 p = make_float2(0.5) + make_float2(up);
            if (in_triangle<H>(p, p0, p1, p2)) {
                uint4 val = make_uint4(tri_idx,
                                       as<uint>(color.x),
                                       as<uint>(color.y),
                                       as<uint>(1.f));
                write(up.x, up.y, val);
            }
        });
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

        float2 p0 = v0.lightmap_uv();
        float2 p1 = v1.lightmap_uv();
        float2 p2 = v2.lightmap_uv();

        if (equal(p0.y, p1.y)) {
            draw_bottom(p0, p1, p2, index);
        } else if (equal(p1.y, p2.y)) {
            draw_top(p0, p1, p2, index);
        } else {
            float factor = float(p1.y - p2.y) / (p0.y - p2.y);
            float2 pc = lerp(factor, p2, p0);
            draw_top(p0, pc, p1, index);
            draw_bottom(pc, p1, p2, index);
        }
        uint4 val = make_uint4(index,
                               as<uint>(1.f),
                               as<uint>(1.f),
                               as<uint>(1.f));
        if (all(p0 == p1) && all(p1 == p2)) {
            return;
        }
        line_bresenham(p0, p1, [&](int x, int y) {
            write(x, y, val);
        });
        line_bresenham(p1, p2, [&](int x, int y) {
            write(x, y, val);
        });
        line_bresenham(p2, p0, [&](int x, int y) {
            write(x, y, val);
        });
    }

    void apply(vision::BakedShape &bs) noexcept override {
        const MergedMesh &mesh = bs.merged_mesh();
        auto &stream = pipeline()->stream();
        vector<uint4> pixels;
        pixels.resize(bs.pixel_num());
        pixels_ = pixels.data();
        res_ = bs.resolution();
        const vector<Vertex> &vertices = mesh.vertices;
        const vector<Triangle> &triangles = mesh.triangles;
        for (uint i = 0; i < triangles.size(); ++i) {
            Triangle tri = triangles.at(i);
            Vertex v0 = vertices.at(tri.i);
            Vertex v1 = vertices.at(tri.j);
            Vertex v2 = vertices.at(tri.k);
            //            draw(v0, v1, v2, i);
            draw_bound(v0.lightmap_uv(), v1.lightmap_uv(), v2.lightmap_uv(), i);
        }
        bs.pixels().upload_immediately(pixels_);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::CPURasterizer)