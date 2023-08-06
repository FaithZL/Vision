//
// Created by Zero on 2023/7/14.
//

#include "importer.h"
#include "mgr/scene.h"
#include "mgr/global.h"

namespace vision {

SP<Importer> Importer::create(const std::string &ext_name) {
    ImporterDesc desc;
    if (ext_name == ".json" || ext_name == ".bson") {
        desc.sub_type = "json";
    } else if (ext_name == ".usda" || ext_name == ".usdc") {
        desc.sub_type = "usd";
    } else {
        desc.sub_type = "assimp";
    }
    return Global::node_mgr().load<Importer>(desc);
}

void Importer::import_scene(const fs::path &fn, Scene *scene) {
    auto importer = Importer::create(fn.extension().string());
    importer->read_file(fn, scene);
}

}// namespace vision