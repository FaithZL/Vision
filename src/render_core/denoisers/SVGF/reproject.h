//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/mgr/global.h"

namespace vision {

class SVGF;

class Reproject : public Ctx {
private:
    SVGF *_svgf{nullptr};

public:
    explicit Reproject(SVGF *svgf)
        : _svgf(svgf) {}

    void prepare() noexcept {

    }

    void compile() noexcept {

    }
};

}// namespace vision