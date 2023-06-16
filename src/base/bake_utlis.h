//
// Created by Zero on 2023/6/2.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"
#include "shape.h"
#include "base/mgr/pipeline.h"
#include "mgr/global.h"
#include "descriptions/json_util.h"

namespace vision {

using namespace ocarina;

struct UVSpreadVertex {
    // Not normalized - values are in Atlas width and height range.
    float2 uv{};
    uint xref{};
};

struct UVSpreadMesh {
    vector<UVSpreadVertex> vertices;
    vector<Triangle> triangles;
};

struct UVSpreadResult {
    uint width;
    uint height;
    vector<UVSpreadMesh> meshes;
};

struct BakedShape {
private:
    Shape *_shape{};
    uint2 _resolution{};
    RegistrableManaged<float4> _normal{Global::instance().pipeline()->resource_array()};
    RegistrableManaged<float4> _position{Global::instance().pipeline()->resource_array()};

public:
    BakedShape() = default;
    explicit BakedShape(Shape *shape) : _shape(shape) {}

    [[nodiscard]] fs::path cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_shape_{:016x}", _shape->hash());
    }

#define VS_MAKE_ATTR_GET(attr, sig) \
    [[nodiscard]] auto sig attr() noexcept { return _##attr; }

    VS_MAKE_ATTR_GET(resolution, )
    VS_MAKE_ATTR_GET(shape, )
    VS_MAKE_ATTR_GET(position, &)
    VS_MAKE_ATTR_GET(normal, &)

#undef VS_MAKE_ATTR_GET

    [[nodiscard]] fs::path uv_config_fn() const noexcept {
        return cache_directory() / "uv_config.json";
    }

    [[nodiscard]] bool has_uv_cache() const noexcept {
        return fs::exists(uv_config_fn());
    }

    [[nodiscard]] size_t pixel_num() const noexcept {
        return _resolution.x * _resolution.y;
    }

    void allocate_device_memory() noexcept {
        _normal.reset_all(_shape->device(), pixel_num());
        _position.reset_all(_shape->device(), pixel_num());
        shape()->for_each_mesh([&](vision::Mesh &mesh, uint index) {

        });
    }

    [[nodiscard]] UVSpreadResult load_uv_spread_result_from_cache() const {
        DataWrap json = create_json_from_file(uv_config_fn());
        auto res = json["resolution"];
        UVSpreadResult spread_result;
        spread_result.width = res[0];
        spread_result.height = res[1];
        _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            DataWrap elm = json["uv_result"][i];
            auto vertices = elm["vertices"];
            UVSpreadMesh u_mesh;
            for (auto vertex : vertices) {
                u_mesh.vertices.emplace_back(make_float2(vertex[0], vertex[1]), vertex[2]);
            }

            auto triangles = elm["triangles"];
            for (auto tri : triangles) {
                u_mesh.triangles.emplace_back(tri[0], tri[1], tri[2]);
            }
            spread_result.meshes.push_back(u_mesh);
        });
        return spread_result;
    }

    void save_to_cache(const UVSpreadResult &result) {
        Context::create_directory_if_necessary(cache_directory());
        DataWrap data = DataWrap::object();
        data["resolution"] = {result.width, result.height};
        data["uv_result"] = DataWrap::array();
        _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            const UVSpreadMesh &u_mesh = result.meshes[i];
            DataWrap elm = DataWrap::object();
            elm["vertices"] = DataWrap::array();
            for (auto vertex : u_mesh.vertices) {
                elm["vertices"].push_back({vertex.uv.x, vertex.uv.y, vertex.xref});
            }
            elm["triangles"] = DataWrap::array();
            for (Triangle tri : u_mesh.triangles) {
                elm["triangles"].push_back({tri.i, tri.j, tri.k});
            }
            data["uv_result"].push_back(elm);
        });

        string data_str = data.dump(4);
        fs::path uv_config = uv_config_fn();
        Context::write_file(uv_config, data_str);
    }

    void remedy_vertices(const UVSpreadResult &result) {
        _resolution = make_uint2(result.width, result.height);
        _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            const UVSpreadMesh &u_mesh = result.meshes[i];
            vector<Vertex> vertices;
            vertices.reserve(u_mesh.vertices.size());
            for (auto &vert : u_mesh.vertices) {
                Vertex vertex = mesh.vertices[vert.xref];
                vertex.set_lightmap_uv(vert.uv / make_float2(result.width, result.height));
                vertices.push_back(vertex);
            }
            mesh.vertices = ocarina::move(vertices);
            mesh.triangles = u_mesh.triangles;
        });
    }
};

class UVSpreader : public Node {
public:
    using Desc = UVSpreaderDesc;

public:
    explicit UVSpreader(const UVSpreaderDesc &desc)
        : Node(desc) {}
    [[nodiscard]] virtual UVSpreadResult apply(const Shape *shape) = 0;
};

class Rasterizer : public Node {
public:
    using Desc = RasterizerDesc;

public:
    explicit Rasterizer(const RasterizerDesc &desc)
        : Node(desc) {}
    virtual void compile_shader() noexcept = 0;
    virtual void apply(BakedShape &baked_shape) noexcept = 0;
};

}// namespace vision