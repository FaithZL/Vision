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

struct UVSpreadResult {
    // Not normalized - values are in Atlas width and height range.
    RegistrableManaged<float2> uv;
    RegistrableManaged<Triangle> triangle;
    Buffer<Vertex> device_vertices;
};

struct BakedShape {
private:
    Shape *_shape{};
    uint2 _resolution{};
    RegistrableManaged<float4> _normal{Global::instance().pipeline()->resource_array()};
    RegistrableManaged<float4> _position{Global::instance().pipeline()->resource_array()};
    vector<UVSpreadResult> _results;

public:
    BakedShape() = default;
    explicit BakedShape(Shape *shape) : _shape(shape) {}
    BakedShape(Shape *shape, uint2 res, vector<UVSpreadResult> datas)
        : _shape(shape), _resolution(res),
          _results(ocarina::move(datas)) {}

    [[nodiscard]] fs::path cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_shape_{:016x}", _shape->hash());
    }

#define VS_MAKE_ATTR_GET(attr, sig) \
    [[nodiscard]] auto sig attr() noexcept { return _##attr; }

    VS_MAKE_ATTR_GET(resolution, )
    VS_MAKE_ATTR_GET(shape, )
    VS_MAKE_ATTR_GET(position, &)
    VS_MAKE_ATTR_GET(normal, &)
    VS_MAKE_ATTR_GET(results, &)

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

    void update_result(vector<UVSpreadResult> results, uint2 res) {
        _results = ocarina::move(results);
        _resolution = res;
    }

    void allocate_device_memory() noexcept {
        _normal.reset_all(_shape->device(), pixel_num());
        _position.reset_all(_shape->device(), pixel_num());
    }

    void load_uv_spread_result_from_cache() {
        DataWrap json = create_json_from_file(uv_config_fn());
        auto res = json["resolution"];
        _resolution = make_uint2(res[0], res[1]);
        _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            DataWrap elm = json["uv_result"][i];
            auto uvs = elm["uv"];

            UVSpreadResult result;
            for (auto uv : uvs) {
                result.uv.push_back(make_float2(uv[0], uv[1]));
            }

            auto triangles = elm["triangle"];
            for (auto tri : triangles) {
                result.triangle.emplace_back(tri[0], tri[1], tri[2]);
            }
            _results.push_back(ocarina::move(result));
        });
    }

    void save_uv_spread_result_to_cache() {
        Context::create_directory_if_necessary(cache_directory());
        DataWrap data = DataWrap::object();
        data["resolution"] = {_resolution.x, _resolution.y};
        data["uv_result"] = DataWrap::array();
        _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            UVSpreadResult &result = _results[i];
            OC_ASSERT(mesh.vertices.size() == result.uv.host_buffer().size());
            OC_ASSERT(mesh.triangles.size() == result.triangle.host_buffer().size());
            DataWrap elm = DataWrap::object();
            elm["uv"] = DataWrap::array();
            for (auto uv : result.uv) {
                elm["uv"].push_back({uv.x, uv.y});
            }
            elm["triangle"] = DataWrap::array();
            for (Triangle tri : result.triangle) {
                elm["triangle"].push_back({tri.i, tri.j, tri.k});
            }
            data["uv_result"].push_back(elm);
        });

        string data_str = data.dump(4);
        fs::path uv_config = uv_config_fn();
        Context::write_file(uv_config, data_str);
    }
};

class UVSpreader : public Node {
public:
    using Desc = UVSpreaderDesc;

public:
    explicit UVSpreader(const UVSpreaderDesc &desc)
        : Node(desc) {}
    virtual void apply(BakedShape &baked_shape) = 0;
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