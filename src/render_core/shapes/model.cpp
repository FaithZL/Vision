//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "importers/assimp_parser.h"

namespace vision {

class Model : public Shape {
private:
    vector<Mesh> _meshes;
    float4x4 _o2w;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        uint64_t ret = Hash64::default_seed;
        for (const Mesh &mesh : _meshes) {
            ret = hash64(mesh.hash(), ret);
        }
        return ret;
    }

public:
    explicit Model(const ShapeDesc &desc)
        : Shape(desc),
          _o2w(desc.o2w.mat) {
        load(desc);
    }

    void set_lightmap_id(ocarina::uint id) noexcept override {
        for (vision::Mesh &mesh : _meshes) {
            mesh.set_lightmap_id(id);
        }
    }

    [[nodiscard]] float4x4 o2w() const noexcept override { return _o2w; }

    void load(const ShapeDesc &desc) noexcept {
        auto fn = scene_path() / desc["fn"].as_string();
        AssimpParser assimp_util;
        assimp_util.load_scene(fn, desc["swap_handed"].as_bool(false),
                               desc["smooth"].as_bool(false),
                               desc["flip_uv"].as_bool(true));
        _meshes = assimp_util.parse_meshes(_material.name.empty(), desc["subdiv_level"].as_uint(0u));
        for (vision::Mesh &mesh : _meshes) {
            mesh._material.name = _material.name;
            mesh._inside = _inside;
            mesh._outside = _outside;
            mesh.handle().o2w = _o2w;
            mesh.handle().light_id = handle().light_id;
            this->aabb.extend(aabb);
        }
    }

    [[nodiscard]] Mesh &mesh_at(ocarina::uint i) noexcept override {
        return _meshes[i];
    }

    [[nodiscard]] const Mesh &mesh_at(ocarina::uint i) const noexcept override {
        return _meshes[i];
    }

    void for_each_mesh(const std::function<void(vision::Mesh &, uint)> &func) noexcept override {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }

    void for_each_mesh(const std::function<void(const vision::Mesh &, uint)> &func) const noexcept override {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)