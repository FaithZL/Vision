//
// Created by Zero on 2023/7/14.
//

#include "base/importer.h"
#include "importers/assimp_parser.h"
#include "base/mgr/pipeline.h"

namespace vision {

class AssimpImporter : public Importer {
private:
    AssimpParser _parser;

public:
    explicit AssimpImporter(const ImporterDesc &desc)
        : Importer(desc) {}

    [[nodiscard]] SP<Pipeline> read_file(const fs::path &fn) override {
        PipelineDesc desc;
        desc.sub_type = "offline";
        SP<Pipeline> ret = Global::node_mgr().load<Pipeline>(desc);
        Global::instance().set_pipeline(ret.get());
        SceneDesc scene_desc;
        scene_desc.init(DataWrap::object());
        Scene &scene = ret->scene();
        scene.init(scene_desc);

        _parser.load_scene(fn);

        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AssimpImporter)