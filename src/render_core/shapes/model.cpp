//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "base/mgr/scene.h"
#include "importers/assimp_parser.h"

namespace vision {

class Model : public ShapeGroup {
private:
    // todo model cache
    struct CacheData {
        vector<std::pair<uint64_t, uint>> meshes;
        vector<uint64_t> materials;
        uint64_t custom_mat;
    };

public:
    explicit Model(const ShapeDesc &desc)
        : ShapeGroup(desc) {
        load(desc);
        post_init(desc);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void load(const ShapeDesc &desc) noexcept {
        auto fn = scene_path() / desc["fn"].as_string();
        AssimpParser parser;
        parser.load_scene(fn, desc["swap_handed"].as_bool(false),
                               desc["smooth"].as_bool(false),
                               desc["flip_uv"].as_bool(true));
        string mat_name = desc["material"].as_string();
        auto instances = parser.parse_meshes(false, desc["subdiv_level"].as_uint(0u));
        add_instances(ocarina::move(instances));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)