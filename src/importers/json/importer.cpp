//
// Created by Zero on 2023/7/14.
//

#include "base/importer.h"
#include "descriptions/json_util.h"
#include "base/mgr/pipeline.h"

namespace vision {

class JsonImporter : public Importer {
private:
public:
    explicit JsonImporter(const ImporterDesc &desc)
        : Importer(desc) {}

    [[nodiscard]] SP<Pipeline> read_file(const fs::path &fn) override {
        auto scene_desc = SceneDesc::from_json(fn);
        SP<Pipeline> ret = Global::node_mgr().load<Pipeline>(scene_desc.pipeline_desc);
        Global::instance().set_pipeline(ret.get());
        ret->init_scene(scene_desc);
        ret->init_postprocessor(scene_desc.denoiser_desc);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::JsonImporter)