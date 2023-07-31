//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "base/mgr/scene.h"
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
        string mat_name = desc["material"].as_string();
        Wrap<Medium> inside;
        Wrap<Medium> outside;
        if (desc.contains("medium")) {
            inside.name = desc["medium"]["inside"].as_string();
            outside.name = desc["medium"]["outside"].as_string();
        } else {
            inside = scene().global_medium();
            outside = scene().global_medium();
        }

        _meshes = assimp_util.parse_meshes(mat_name.empty(), desc["subdiv_level"].as_uint(0u));
        for (SP<Mesh> mesh : _meshes) {
            mesh->_material.name = mat_name;
            mesh->_inside = inside;
            mesh->_outside = outside;
            mesh->handle().o2w = _o2w;
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)