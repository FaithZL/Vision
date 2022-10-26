//
// Created by Zero on 02/09/2022.
//

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "core/context.h"
#include "util/image_io.h"
#include "core/logging.h"
#include "application.h"

using namespace ocarina;
using namespace vision;

int execute(int argc, char *argv[]){
    App app(argc, argv);
    if (argc == 1) {
        app.context.cli_parser().print_help();
        return 0;
    }
    app.prepare();
    return app.run();;
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}