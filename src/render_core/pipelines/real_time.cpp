//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class RealTimeRenderPipeline : public Pipeline {
public:
    explicit RealTimeRenderPipeline(const PipelineDesc &desc)
        : Pipeline(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeRenderPipeline)