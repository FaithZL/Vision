//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "importers/assimp_parser.h"

namespace vision {

class Model : public Group {
public:
    explicit Model(const ShapeDesc &desc)
        : Group(desc) {
        load(desc);
    }

    void load(const ShapeDesc &desc) noexcept {
        auto fn = scene_path() / desc["fn"].as_string();
        AssimpParser assimp_util;
        assimp_util.load_scene(fn, desc["swap_handed"].as_bool(false),
                               desc["smooth"].as_bool(false),
                               desc["flip_uv"].as_bool(true));
        _meshes = assimp_util.parse_meshes(_material.name.empty(), desc["subdiv_level"].as_uint(0u));
        for (SP<Mesh> mesh : _meshes) {
            mesh->_material.name = _material.name;
            mesh->_inside = _inside;
            mesh->_outside = _outside;
            mesh->handle().o2w = _o2w;
            mesh->handle().light_id = handle().light_id;
            this->aabb.extend(aabb);
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)