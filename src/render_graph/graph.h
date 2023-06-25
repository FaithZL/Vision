//
// Created by Zero on 2023/6/25.
//

#pragma once

#include "rhi/common.h"
#include "graph_compiler.h"
#include "pass.h"

namespace vision {

class RenderGraph {
private:
    RenderGraphCompiler _compiler;


public:
    void add_pass(const SP<RenderPass> &pass) noexcept;
    void add_connection(const string &output, const string &input) noexcept;

    void setup() noexcept;
    void compile() noexcept;
    void execute() noexcept;
};

}