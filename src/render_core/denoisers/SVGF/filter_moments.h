//
// Created by Zero on 2024/2/12.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"
#include "data.h"

namespace vision {

class SVGF;

class FilterMoments : public Context {
private:
    SVGF *_svgf{nullptr};
    using signature = void(Buffer<SVGFData>, Buffer<PixelGeometry>,
                           float, float);
    Shader<signature> _shader;

public:
    explicit FilterMoments(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(RealTimeDenoiseInput &input) noexcept;
};

}// namespace vision
