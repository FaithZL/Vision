//
// Created by Zero on 2024/2/10.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/sensor/filter.h"
#include "base/denoiser.h"
#include "base/mgr/node_mgr.h"

namespace vision {
using namespace ocarina;

class AtrousFilter  {
private:
    SP<Filter> _filter;

public:
    explicit AtrousFilter(const Filter::Desc &desc)
        : _filter(NodeMgr::instance().load<Filter>(desc)) {}

};

}// namespace vision