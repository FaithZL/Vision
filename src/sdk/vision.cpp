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
    Pipeline *_pipeline{};

public:
    void init_pipeline() override {
    }

    void init_scene() override {
    }

    void add_instance(Instance instance) override {
    }

    void build_accel() override {
    }

    void update_camera(vision::sdk::Camera camera) override {
    }

    void update_resolution(uint32_t width, uint32_t height) override {
    }
};

}// namespace vision::sdk

VS_EXPORT_API vision::sdk::VisionRenderer *create() {
    return ocarina::new_with_allocator<vision::sdk::VisionRendererImpl>();
}
OC_EXPORT_API void destroy(vision::sdk::VisionRenderer *obj) {
    ocarina::delete_with_allocator(obj);
}
