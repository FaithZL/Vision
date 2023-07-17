//
// Created by Zero on 2023/7/17.
//

#include "vision.h"
#include "base/node.h"
#include "core/stl.h"

namespace vision::sdk {

class VisionRendererImpl : public VisionRenderer {
private:
public:
    void init_device() override {
    }

    void add_instance(Instance instance) override {
    }

    void build_accel() override {
    }
};

}// namespace vision::sdk

VS_EXPORT_API vision::sdk::VisionRenderer *create() {
    return ocarina::new_with_allocator<vision::sdk::VisionRendererImpl>();
}
OC_EXPORT_API void destroy(vision::sdk::VisionRenderer *obj) {
    ocarina::delete_with_allocator(obj);
}
