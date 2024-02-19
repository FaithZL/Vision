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

namespace vision {
using namespace ocarina;

class SVGF;

class AtrousFilter : public Context {
private:
    SP<Filter> _filter;
    SVGF *_svgf{nullptr};


public:
    explicit AtrousFilter(const Filter::Desc &desc, SVGF *svgf)
        : _filter(NodeMgr::instance().load<Filter>(desc)), _svgf(svgf) {}
    void prepare() noexcept;
    void compile() noexcept;
    [[nodiscard]] CommandList dispatch(vision::RealTimeDenoiseInput &input, uint step_width) noexcept;
};

}// namespace vision