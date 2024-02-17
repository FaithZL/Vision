//
// Created by Zero on 2024/2/12.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"
#include "base/denoiser.h"

namespace vision {

class SVGF;

class FilterMoment : public Context {
private:
    SVGF *_svgf{nullptr};

public:
    explicit FilterMoment(SVGF *svgf)
        : _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(DenoiseInput &input) noexcept;
};

}// namespace vision
