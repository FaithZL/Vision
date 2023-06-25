//
// Created by Zero on 2023/6/24.
//

#pragma once

#include "rhi/common.h"
#include "base/node.h"
#include "resource.h"

namespace vision {

using namespace ocarina;

class RenderPass : public Node {
private:
    std::map<string, RenderResource> _input_map;
    std::map<string, RenderResource> _output_map;

public:
    RenderPass() = default;

    virtual void setup() noexcept = 0;
    virtual void compile() noexcept = 0;
    virtual void execute() noexcept = 0;
};

}// namespace vision