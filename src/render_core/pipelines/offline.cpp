//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class OfflineRenderPipeline : public Pipeline {
public:
    explicit OfflineRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}

    void prepare() noexcept override {
        auto pixel_num = resolution().x * resolution().y;
        _final_picture.reset_all(device(), pixel_num);
        _scene.prepare();
        image_pool().prepare();
        prepare_geometry();
        prepare_resource_array();
        compile_shaders();
        preprocess();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OfflineRenderPipeline)