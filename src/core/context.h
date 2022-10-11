//
// Created by Zero on 05/10/2022.
//

#pragma once

#include "core/stl.h"
#include "rhi/context.h"
#include "description/descriptions.h"
#include "cli_parser.h"
#include "base/scene_node.h"

namespace vision {

//class SceneNode;

using namespace ocarina;

class Context : public ocarina::Context {
public:
    using Super = ocarina::Context;
    using PluginHandle = ocarina::unique_ptr<SceneNode, SceneNode::Deleter *>;

private:
    CLIParser _cli_parser;

public:
    explicit Context(int argc, char **argv,
                     ocarina::string_view cache_dir = ".cache");
    const CLIParser &cli_parser() const noexcept { return _cli_parser; }
    [[nodiscard]] PluginHandle load_plugin(Description *desc);

};

}// namespace vision