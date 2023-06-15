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
    vector<float3> _pos;

public:
    explicit XAtlas(const UVSpreaderDesc &desc)
        : UVSpreader(desc),
          _padding(desc["padding"].as_uint(3)),
          _atlas(nullptr) {
    }

    struct Guard {
        XAtlas *spreader{};
        explicit Guard(XAtlas *spreader) : spreader(spreader) { spreader->init_xatlas(); }
        ~Guard() { spreader->destroy_xatlas(); }
    };

    void destroy_xatlas() {
        if (_atlas) {
            xatlas::Destroy(_atlas);
            _atlas = nullptr;
        }
    }

    void init_xatlas() {
        _atlas = xatlas::Create();
        xatlas::SetProgressCallback(_atlas, progress_callback, &_stopwatch);
    }

    [[nodiscard]] static xatlas::MeshDecl mesh_decl(vision::Mesh *mesh) {
        xatlas::MeshDecl ret;
        vector<Vertex> &vertices = mesh->vertices;
        vector<Triangle> &triangle = mesh->triangles;

        // fill position
        ret.vertexCount = vertices.size();
        ret.vertexPositionData = vertices.data();
        ret.vertexPositionStride = sizeof(Vertex);

        // fill normal
        ret.vertexNormalData = reinterpret_cast<std::byte *>(vertices.data()) + Vertex::n_offset();
        ret.vertexNormalStride = sizeof(Vertex);

        // fill normal
        ret.vertexNormalData = reinterpret_cast<std::byte *>(vertices.data()) + Vertex::n_offset();
        ret.vertexNormalStride = sizeof(Vertex);

        // fill tex_coord
        ret.vertexUvData = reinterpret_cast<std::byte *>(vertices.data()) + Vertex::uv_offset();
        ret.vertexUvStride = sizeof(Vertex);

        // fill indices
        ret.indexCount = triangle.size() * 3;
        ret.indexData = triangle.data();
        ret.indexFormat = xatlas::IndexFormat::UInt32;

        OC_INFO_FORMAT("xatlas mesh decl vertex num is {}, triangle num is {}", vertices.size(), triangle.size());

        return ret;
    }

    [[nodiscard]] xatlas::PackOptions pack_options() const noexcept {
        xatlas::PackOptions ret;
        ret.padding = _padding;
        return ret;
    }

    [[nodiscard]] xatlas::ChartOptions chart_options() const noexcept {
        xatlas::ChartOptions ret;
        return ret;
    }

    void apply(vision::Shape *shape) override {
        Guard __(this);
        vision::Mesh *mesh = dynamic_cast<vision::Mesh *>(shape);
        xatlas::MeshDecl decl = mesh_decl(mesh);

        xatlas::AddMeshError error = xatlas::AddMesh(_atlas, decl, 1);
        //         error = xatlas::AddMesh(_atlas, decl, 1);
        if (error != xatlas::AddMeshError::Success) {
            destroy_xatlas();
            OC_ERROR("xatlas adding mesh error");
        }

        xatlas::AddMeshJoin(_atlas);
        auto &a = *_atlas;
        xatlas::Generate(_atlas, chart_options(), pack_options());
        a = *_atlas;
        auto &m = *a.meshes;
        m = *a.meshes;

        vector<xatlas::Vertex> vert;
        for (int i = 0; i < m.vertexCount; ++i) {
            auto &v = m.vertexArray[i];
            v.uv[0] /= a.width;
            v.uv[1] /= a.height;
            vert.push_back(m.vertexArray[i]);
        }

        vector<uint> indices;
        for (int i = 0; i < m.indexCount; ++i) {
            indices.push_back(m.indexArray[i]);
        }

        uint2 res = make_uint2(_atlas->width, _atlas->height);

    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::XAtlas)