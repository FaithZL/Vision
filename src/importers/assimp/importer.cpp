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

    [[nodiscard]] vector<SP<ShapeGroup>> parse_shapes() noexcept {
        vector<SP<ShapeGroup>> ret;
        vector<ShapeInstance> instances = _parser.parse_meshes(true, 0);
        for (const auto &inst : instances) {
            SP<ShapeGroup> group = make_shared<ShapeGroup>(inst);
            ret.push_back(group);
        }
        return ret;
    }

    [[nodiscard]] SP<Pipeline> read_file(const fs::path &fn) override {
        PipelineDesc desc;
        desc.sub_type = "fixed";
        SP<Pipeline> ret = Global::node_mgr().load<Pipeline>(desc);
        Global::instance().set_pipeline(ret.get());
        SceneDesc scene_desc;
        scene_desc.init(DataWrap::object());
        Scene &scene = ret->scene();
        scene.init(scene_desc);

        _parser.load_scene(fn);

        auto shapes = parse_shapes();
        std::for_each(shapes.begin(), shapes.end(), [&](const SP<ShapeGroup>& shape) {
            scene.add_shape(shape);
            scene.add_material(shape->instance(0).material());
        });

        auto lights = _parser.parse_lights();
        std::for_each(lights.begin(), lights.end(), [&](SP<Light> light) {
            scene.add_light(ocarina::move(light));
        });

        auto cameras = _parser.parse_cameras();
        scene.camera()->update_mat(cameras[0]);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AssimpImporter)