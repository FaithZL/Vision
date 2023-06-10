//
// Created by Zero on 2023/6/1.
//

#include "postprocessor.h"
#include "render_pipeline.h"

namespace vision {

Postprocessor::Postprocessor(RenderPipeline *rp)
    : _rp(rp) {}
}// namespace vision