//
// Created by Zero on 2023/6/24.
//

#pragma once

#include "rhi/common.h"

namespace vision {

using namespace ocarina;

class RenderPass {
public:
    virtual void setup() noexcept = 0;
    virtual void compile() noexcept = 0;
    virtual void execute() noexcept = 0;
};

}// namespace vision