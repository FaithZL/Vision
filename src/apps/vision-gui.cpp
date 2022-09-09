//
// Created by Zero on 02/09/2022.
//

#include <iostream>
#include "core/cli_parser.h"
#include "ocarina/src/core/stl.h"

using namespace ocarina;

int main(int argc, char *argv[]) {
    fs::path path(argv[0]);

    vision::CLIParser cli_parser(argc, argv);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}