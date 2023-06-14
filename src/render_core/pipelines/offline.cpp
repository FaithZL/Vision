//
// Created by Zero on 2023/6/12.
//

#include "base/mgr/pipeline.h"

namespace vision {

class OfflineRenderPipeline : public Pipeline {
public:
    explicit OfflineRenderPipeline(const PipelineDesc &desc) : Pipeline(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OfflineRenderPipeline)