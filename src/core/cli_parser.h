//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "cxxopts.hpp"
#include "ocarina/src/core/stl.h"

namespace vision {

class CLIParser {
private:
    mutable cxxopts::Options _cli_options;

public:
    CLIParser(int argc, char **argv);
    void init(int argc, char **argv);
};

}// namespace vision
