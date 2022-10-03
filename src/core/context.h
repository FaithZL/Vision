//
// Created by Zero on 05/10/2022.
//

#pragma once

#include "core/stl.h"
#include "rhi/context.h"
#include "cli_parser.h"

namespace vision {

class Context : public ocarina::Context {
public:
    using Super = ocarina::Context;

private:
    CLIParser _cli_parser;

public:
    explicit Context(int argc, char **argv,
                     ocarina::string_view cache_dir = ".cache");
    const CLIParser &cli_parser() const noexcept { return _cli_parser; }


};

}// namespace vision