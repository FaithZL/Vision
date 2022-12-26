//
// Created by Zero on 26/12/2022.
//

#pragma once

#include "dsl/common.h"
#include "rhi/common.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

using namespace ocarina;

class SPD {
private:
    Managed<float> _func;
    RenderPipeline *_rp{};

public:
    static SPD create_cie_x(RenderPipeline *rp);
    static SPD create_cie_y(RenderPipeline *rp);
    static SPD create_cie_z(RenderPipeline *rp);
    static SPD create_cie_d65(RenderPipeline *rp);
};

}// namespace vision