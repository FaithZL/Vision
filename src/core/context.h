//
// Created by Zero on 05/10/2022.
//

#pragma once

#include "core/stl.h"
#include "rhi/context.h"
#include "descriptions/node_desc.h"
#include "cli_parser.h"
#include "base/node.h"

namespace vision {

//class SceneNode;

using namespace ocarina;

class Context : public ocarina::Context {
public:
    using Super = ocarina::Context;

private:
    CLIParser _cli_parser;

public:
    explicit Context(int argc, char **argv,
                     ocarina::string_view cache_dir = ".cache");
    const CLIParser &cli_parser() const noexcept { return _cli_parser; }
    [[nodiscard]] Node::Handle load_plugin(NodeDesc *desc);

};

}// namespace vision