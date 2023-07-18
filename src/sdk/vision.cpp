//
// Created by Zero on 2023/7/17.
//

#include "vision.h"
#include "base/node.h"
#include "core/stl.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision::sdk {

class VisionRendererImpl : public VisionRenderer {
private:
    UP<Device> _device;
    Pipeline *_pipeline{};

public:
    void init_pipeline(const char *rpath) override;
    void init_scene() override;
    void add_instance(Instance instance) override;
    void build_accel() override;
    void update_camera(vision::sdk::Camera camera) override;
    void update_resolution(uint32_t width, uint32_t height) override;
};

void VisionRendererImpl::init_pipeline(const char *rpath) {
    PipelineDesc desc;
    Context::instance().init(rpath);
    _device = make_unique<Device>(Context::instance().create_device("cuda"));
    desc.device = _device.get();
    desc.sub_type = "offline";
    _pipeline = Global::node_mgr().load<Pipeline>(desc);
    Global::instance().set_pipeline(_pipeline);
}

const char *integrator_param = R"(
{
    "type": "pt",
    "param" : {

    }
}
)";

void VisionRendererImpl::init_scene() {
    SceneDesc scene_desc;
    scene_desc.init(DataWrap::object());
    Scene &scene = _pipeline->scene();
    scene.init(scene_desc);
    int i = 0;
}

void VisionRendererImpl::add_instance(vision::sdk::Instance instance) {

}

void VisionRendererImpl::build_accel() {

}

void VisionRendererImpl::update_camera(vision::sdk::Camera camera) {

}

void VisionRendererImpl::update_resolution(uint32_t width, uint32_t height) {

}

}// namespace vision::sdk

VS_EXPORT_API vision::sdk::VisionRenderer *create() {
    return ocarina::new_with_allocator<vision::sdk::VisionRendererImpl>();
}
OC_EXPORT_API void destroy(vision::sdk::VisionRenderer *obj) {
    ocarina::delete_with_allocator(obj);
}
