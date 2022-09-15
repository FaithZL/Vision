//
// Created by Zero on 02/09/2022.
//

#include <iostream>
#include "core/cli_parser.h"
#include "loader/scene_desc.h"
#include "core/stl.h"

using namespace ocarina;

int execute(int argc, char *argv[]){
    fs::path path(argv[0]);
    vision::CLIParser cli_parser(argc, argv);
    if (argc == 1) {
        cli_parser.print_help();
        return 0;
    }


    return 0;
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}