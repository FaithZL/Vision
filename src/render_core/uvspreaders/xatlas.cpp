//
// Created by Zero on 2023/6/2.
//

#include "base/uv_spreader.h"
#include "ext/xatlas/xatlas.h"

namespace vision {
using namespace ocarina;

class Stopwatch {
public:
    Stopwatch() { reset(); }
    void reset() { m_start = clock(); }
    double elapsed() const { return (clock() - m_start) * 1000.0 / CLOCKS_PER_SEC; }

private:
    clock_t m_start;
};

static void RandomColor(uint8_t *color)
{
    for (int i = 0; i < 3; i++)
        color[i] = uint8_t((rand() % 255 + 192) * 0.5f);
}

static void SetPixel(uint8_t *dest, int destWidth, int x, int y, const uint8_t *color)
{
    uint8_t *pixel = &dest[x * 3 + y * (destWidth * 3)];
    pixel[0] = color[0];
    pixel[1] = color[1];
    pixel[2] = color[2];
}

// https://github.com/miloyip/line/blob/master/line_bresenham.c
// License: public domain.
static void RasterizeLine(uint8_t *dest, int destWidth, const int *p1, const int *p2, const uint8_t *color)
{
    const int dx = std::abs(p2[0] - p1[0]), sx = p1[0] < p2[0] ? 1 : -1;
    const int dy = std::abs(p2[1] - p1[1]), sy = p1[1] < p2[1] ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int current[2];
    current[0] = p1[0];
    current[1] = p1[1];
    while (SetPixel(dest, destWidth, current[0], current[1], color), current[0] != p2[0] || current[1] != p2[1])
    {
        const int e2 = err;
        if (e2 > -dx) { err -= dy; current[0] += sx; }
        if (e2 < dy) { err += dx; current[1] += sy; }
    }
}

/*
https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
Copyright Dmitry V. Sokolov

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
static void RasterizeTriangle(uint8_t *dest, int destWidth, const int *t0, const int *t1, const int *t2, const uint8_t *color)
{
    if (t0[1] > t1[1]) std::swap(t0, t1);
    if (t0[1] > t2[1]) std::swap(t0, t2);
    if (t1[1] > t2[1]) std::swap(t1, t2);
    int total_height = t2[1] - t0[1];
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1[1] - t0[1] || t1[1] == t0[1];
        int segment_height = second_half ? t2[1] - t1[1] : t1[1] - t0[1];
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1[1] - t0[1] : 0)) / segment_height;
        int A[2], B[2];
        for (int j = 0; j < 2; j++) {
            A[j] = int(t0[j] + (t2[j] - t0[j]) * alpha);
            B[j] = int(second_half ? t1[j] + (t2[j] - t1[j]) * beta : t0[j] + (t1[j] - t0[j]) * beta);
        }
        if (A[0] > B[0]) std::swap(A, B);
        for (int j = A[0]; j <= B[0]; j++)
            SetPixel(dest, destWidth, j, t0[1] + i, color);
    }
}


static bool progress_callback(xatlas::ProgressCategory category, int progress, void *userData) {
    // Don't interupt verbose printing.
    Stopwatch *stopwatch = (Stopwatch *)userData;
    static std::mutex progressMutex;
    std::unique_lock<std::mutex> lock(progressMutex);
    if (progress == 0) {
        stopwatch->reset();
    }
    OC_INFO_FORMAT("{} {} %", xatlas::StringForEnum(category), progress);
    if (progress == 100) {
        OC_INFO_FORMAT("{} seconds ({} ms) elapsed", stopwatch->elapsed() / 1000.0, stopwatch->elapsed());
    }
    return true;
}

class XAtlas : public UVSpreader {
private:
    xatlas::Atlas *_atlas{};
    Stopwatch _global_stopwatch;
    Stopwatch _stopwatch;
    uint _padding{};

public:
    explicit XAtlas(const UVSpreaderDesc &desc)
        : UVSpreader(desc),
          _padding(desc["padding"].as_uint(3)),
          _atlas(xatlas::Create()) {
        init_xatlas();
    }

    void destroy_xatlas() {
        if (_atlas) {
            xatlas::Destroy(_atlas);
            _atlas = nullptr;
        }
    }

    void init_xatlas() {
        xatlas::SetProgressCallback(_atlas, progress_callback, &_stopwatch);
    }

    [[nodiscard]] static xatlas::MeshDecl mesh_decl(vision::Mesh *mesh) {
        xatlas::MeshDecl ret;
        vector<Vertex> &vertices = mesh->vertices;
        vector<Triangle> &triangle = mesh->triangles;

        // fill position
        ret.vertexCount = vertices.size();
        ret.vertexPositionData = vertices.data();
        ret.vertexPositionStride = Vertex::pos_stride();

        // fill normal
        ret.vertexNormalData = reinterpret_cast<std::byte *>(vertices.data()) + Vertex::n_offset();
        ret.vertexNormalStride = Vertex::n_stride();

        // fill tex_coord
        ret.vertexUvData = reinterpret_cast<std::byte *>(vertices.data()) + Vertex::uv_offset();
        ret.vertexUvStride = Vertex::uv_stride();

        // fill indices
        ret.indexCount = triangle.size() * 3;
        ret.indexData = triangle.data();
        ret.indexFormat = xatlas::IndexFormat::UInt32;

        OC_INFO_FORMAT("xatlas mesh decl vertex num is {}, triangle num is {}", vertices.size(), triangle.size());

        return ret;
    }

    [[nodiscard]] xatlas::PackOptions pack_options() const noexcept {
        xatlas::PackOptions ret;
        ret.createImage = true;
        ret.padding = _padding;
//        ret.bruteForce = true;
        return ret;
    }

    [[nodiscard]] xatlas::ChartOptions chart_options() const noexcept {
        xatlas::ChartOptions ret;
        return ret;
    }

    void apply(vision::Shape *mesh) override {
        return ;
//        xatlas::MeshDecl decl = mesh_decl(mesh);
//
//        xatlas::AddMeshError error = xatlas::AddMesh(_atlas, decl, 1);
//        if (error != xatlas::AddMeshError::Success) {
//            destroy_xatlas();
//            OC_ERROR("xatlas adding mesh error");
//        }
//
//        xatlas::AddMeshJoin(_atlas);
//        auto &a = *_atlas;
//        xatlas::Generate(_atlas, chart_options(), pack_options());
//        a = *_atlas;
//        auto &m = *a.meshes;
//        m = *a.meshes;
//
//        vector<xatlas::Vertex> vert;
//        for (int i = 0; i < m.vertexCount; ++i) {
//            auto &v = m.vertexArray[i];
//            v.uv[0] /= a.width;
//            v.uv[1] /= a.height;
//            vert.push_back(m.vertexArray[i]);
//        }
//
//        vector<uint> indices;
//        for (int i = 0; i < m.indexCount; ++i) {
//            indices.push_back(m.indexArray[i]);
//        }
//
//        uint2 res = make_uint2(_atlas->width, _atlas->height);
//
//
//
//        mesh->allocate_lightmap_uv();
    }

    ~XAtlas() override {
        destroy_xatlas();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::XAtlas)