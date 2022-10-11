//
// Created by Zero on 02/09/2022.
//

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "rhi/context.h"

using namespace ocarina;
using namespace vision;

int execute(int argc, char *argv[]){
    fs::path path(argv[0]);
    vision::CLIParser cli_parser(argc, argv);
    Context context(path.parent_path());
    if (argc == 1) {
        cli_parser.print_help();
        return 0;
    }

    auto scene_desc = SceneDesc::from_json(cli_parser.scene_file());


    return 0;
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}