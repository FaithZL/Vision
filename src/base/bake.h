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
    vector<float2> uv;
    vector<Triangle> triangle;
};

struct BakedShape {
public:
    Shape *shape{};
    uint2 resolution{};
    RegistrableManaged<float4> normal{Global::instance().pipeline()->resource_array()};
    RegistrableManaged<float4> position{Global::instance().pipeline()->resource_array()};
    vector<UVSpreadResult> results;

public:
    BakedShape() = default;
    explicit BakedShape(Shape *shape) : shape(shape) {}
    BakedShape(Shape *shape, uint2 res, vector<UVSpreadResult> datas)
        : shape(shape), resolution(res), results(ocarina::move(datas)) {}

    [[nodiscard]] fs::path cache_directory() const noexcept {
        return Global::instance().scene_cache_path() / ocarina::format("baked_shape_{:016x}", shape->hash());
    }

    [[nodiscard]] fs::path uv_config_fn() const noexcept {
        return cache_directory() / "uv_config.json";
    }

    [[nodiscard]] bool has_uv_cache() const noexcept {
        return fs::exists(uv_config_fn());
    }

    [[nodiscard]] size_t pixel_num() const noexcept {
        return resolution.x * resolution.y;
    }

    void allocate_normal() noexcept {
        normal.reset_all(shape->device(), pixel_num());
    }

    void allocate_position() noexcept {
        position.reset_all(shape->device(), pixel_num());
    }

    void allocate_maps() noexcept {
        allocate_normal();
        allocate_position();
    }

    void load_uv_spread_result_from_cache() {
        DataWrap json = create_json_from_file(uv_config_fn());
        auto res = json["resolution"];
        resolution = make_uint2(res[0], res[1]);
        shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
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
            results.push_back(result);
        });
    }

    void save_uv_spread_result_to_cache() {
        Context::create_directory_if_necessary(cache_directory());
        DataWrap data = DataWrap::object();
        data["resolution"] = {resolution.x, resolution.y};
        data["uv_result"] = DataWrap::array();
        shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            UVSpreadResult result = results[i];
            OC_ASSERT(mesh.vertices.size() == result.uv.size());
            OC_ASSERT(mesh.triangles.size() == result.triangle.size());
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