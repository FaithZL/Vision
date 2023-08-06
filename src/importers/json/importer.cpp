//
// Created by Zero on 2023/7/14.
//

#include "base/importer.h"
#include "descriptions/json_util.h"

namespace vision {

class JsonImporter : public Importer {
private:
public:
    explicit JsonImporter(const ImporterDesc &desc)
        : Importer(desc) {}

    void read_file(const fs::path &fn, Scene *scene) override {
        auto scene_desc = SceneDesc::from_json(fn);
        scene->init(scene_desc);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::JsonImporter)