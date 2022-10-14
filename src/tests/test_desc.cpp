//
// Created by Zero on 14/10/2022.
//

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "descriptions/desc.h"
#include "core/context.h"

using namespace ocarina;
using namespace vision;

int execute(int argc, char *argv[]) {
    vision::Context context(argc, argv);
    if (argc == 1) {
        context.cli_parser().print_help();
        return 0;
    }

    auto vv = Desc::value_ty("");



    return 0;
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}