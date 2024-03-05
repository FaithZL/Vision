//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/sensor/filter.h"
#include "base/denoiser.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"
#include "utils.h"

namespace vision {
using namespace ocarina;

class SVGF;

class AtrousFilter : public Context {
private:
    SVGF *_svgf{nullptr};

    using signature = void(Buffer<SVGFData>, Buffer<PixelGeometry>, Buffer<float>, float, float, int);
    Shader<signature> _shader;

public:
    explicit AtrousFilter(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(vision::RealTimeDenoiseInput &input, uint step_width) noexcept;
};

}// namespace vision