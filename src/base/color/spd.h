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
    float _sample_interval{};
    RenderPipeline *_rp{};

public:
    SPD(vector<float> func, float interval, RenderPipeline *rp);
    [[nodiscard]] Float sample(const Float& lambda) const noexcept;
    [[nodiscard]] static SPD create_cie_x(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_y(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_z(RenderPipeline *rp) noexcept;
    [[nodiscard]] static SPD create_cie_d65(RenderPipeline *rp) noexcept;
    [[nodiscard]] static float cie_y_integral() noexcept;
};

}// namespace vision