//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class BakerPipeline : public Pipeline {
public:
    explicit BakerPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}

    void preprocess() noexcept override {

    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::BakerPipeline)