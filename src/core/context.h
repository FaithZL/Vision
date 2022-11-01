//
// Created by Zero on 05/10/2022.
//

#pragma once

#include "core/stl.h"
#include "rhi/context.h"
#include "descriptions/node_desc.h"
#include "cli_parser.h"
#include "base/node.h"
#include "base/scene.h"
#include "descriptions/node_desc.h"
#include "render_pipeline.h"

namespace vision {

using namespace ocarina;

class Filter;

class Context : public ocarina::Context {
public:
    using Super = ocarina::Context;

private:
    CLIParser _cli_parser;

public:
    explicit Context(int argc, char **argv,
                     ocarina::string_view cache_dir = ".cache");
    [[nodiscard]] SceneDesc parse_file() const noexcept;
    [[nodiscard]] fs::path scene_directory() const noexcept;
    [[nodiscard]] RenderPipeline create_pipeline(Device *device) { return {device, this}; }
    [[nodiscard]] const CLIParser &cli_parser() const noexcept { return _cli_parser; }
    [[nodiscard]] CLIParser &cli_parser() noexcept { return _cli_parser; }
};

}// namespace vision