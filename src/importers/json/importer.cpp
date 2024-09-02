//
// Created by Zero on 2023/7/14.
//

#include "base/importer.h"
#include "core/json_util.h"
#include "base/mgr/pipeline.h"

namespace vision {

class JsonImporter : public Importer {
public:
    explicit JsonImporter(const ImporterDesc &desc)
        : Importer(desc) {}

    [[nodiscard]] SP<Pipeline> read_file(const fs::path &fn) override {
        auto scene_desc = SceneDesc::from_json(fn);
        SP<Pipeline> ret = Node::load_shared<Pipeline>(scene_desc.pipeline_desc);
        ret->init_scene(scene_desc);
        ret->init_postprocessor(scene_desc.denoiser_desc);
        ret->output_desc = scene_desc.output_desc;
        return ret;
    }
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::JsonImporter)