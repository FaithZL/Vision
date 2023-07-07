//
// Created by Zero on 2023/6/17.
//

#include "bake_utlis.h"

namespace vision {

UnwrapperResult BakedShape::load_uv_config_from_cache() const {
    DataWrap json = create_json_from_file(uv_config_fn());
    auto res = json["resolution"];
    UnwrapperResult spread_result;
    spread_result.width = res[0];
    spread_result.height = res[1];
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        DataWrap elm = json["uv_result"][i];
        auto vertices = elm["vertices"];
        UnwrapperMesh u_mesh;
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

void BakedShape::save_to_cache(const UnwrapperResult &result) {
    Context::create_directory_if_necessary(cache_directory());
    DataWrap data = DataWrap::object();
    data["resolution"] = {result.width, result.height};
    data["uv_result"] = DataWrap::array();
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        const UnwrapperMesh &u_mesh = result.meshes[i];
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

CommandList BakedShape::save_lightmap_to_cache() const {
    CommandList ret;
    float4 *ptr = ocarina::allocate<float4>(pixel_num());
    ret << _lightmap_tex.download(ptr);
    ret << [&, ptr] {
        ImageIO::save_image(lightmap_cache_path(), PixelStorage::FLOAT4, _resolution, ptr);
        ocarina::deallocate(ptr);
    };
    return ret;
}

void BakedShape::allocate_lightmap_texture() noexcept {
    _lightmap_tex = shape()->device().create_texture(resolution(), ocarina::PixelStorage::FLOAT4);
}

void BakedShape::setup_vertices(UnwrapperResult result) {
    _resolution = make_uint2(result.width, result.height);
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        UnwrapperMesh &u_mesh = result.meshes[i];
        vector<Vertex> vertices;
        vertices.reserve(u_mesh.vertices.size());
        for (auto &vert : u_mesh.vertices) {
            Vertex vertex = mesh.vertices[vert.xref];
            vertex.set_lightmap_uv(vert.uv);
            vertices.push_back(vertex);
        }
        mesh.vertices = ocarina::move(vertices);
        mesh.triangles = ocarina::move(u_mesh.triangles);
    });
}

void BakedShape::normalize_lightmap_uv() {
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        for (auto &vert : mesh.vertices) {
            vert.set_lightmap_uv(vert.lightmap_uv() / make_float2(resolution()));
        }
    });
}

uint64_t BakedShape::instance_hash() const noexcept {
    return hash64(_shape->hash(), _shape->o2w());
}

fs::path BakedShape::uv_config_fn() const noexcept {
    return cache_directory() / "uv_config.json";
}

bool BakedShape::has_uv_cache() const noexcept {
    return fs::exists(uv_config_fn());
}

fs::path BakedShape::lightmap_cache_path() const noexcept {
    return instance_cache_directory() / "lightmap.exr";
}

}// namespace vision