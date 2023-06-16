//
// Created by Zero on 2023/6/2.
//

#include "base/bake_utlis.h"
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

    [[nodiscard]] static xatlas::MeshDecl mesh_decl(vision::Mesh &mesh) {
        xatlas::MeshDecl ret;
        vector<Vertex> &vertices = mesh.vertices;
        vector<Triangle> &triangle = mesh.triangles;

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

    void apply(BakedShape &baked_shape) override {
        Guard __(this);

        baked_shape.shape()->for_each_mesh([&](vision::Mesh &mesh, uint) {
            xatlas::MeshDecl decl = mesh_decl(mesh);
            xatlas::AddMeshError error = xatlas::AddMesh(_atlas, decl, 1);
            if (error != xatlas::AddMeshError::Success) {
                destroy_xatlas();
                OC_ERROR("xatlas adding mesh error");
            }
        });

        xatlas::AddMeshJoin(_atlas);
        xatlas::Generate(_atlas, chart_options(), pack_options());

        vector<UVSpreadResult> results;

        for (int i = 0; i < _atlas->meshCount; ++i) {
            UVSpreadResult result;
            xatlas::Mesh &mesh = _atlas->meshes[i];
            for (int j = 0; j < mesh.vertexCount; ++j) {
                xatlas::Vertex &vertex = mesh.vertexArray[j];
                result.uv.push_back(make_float2(vertex.uv[0], vertex.uv[1]));
            }

            for (int j = 0; j < mesh.indexCount; j += 3) {
                uint i0 = mesh.indexArray[j];
                uint i1 = mesh.indexArray[j + 1];
                uint i2 = mesh.indexArray[j + 2];
                result.triangle.emplace_back(i0, i1, i2);
            }
            results.push_back(ocarina::move(result));
        }
        baked_shape.update_result(ocarina::move(results),make_uint2(_atlas->width, _atlas->height));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::XAtlas)