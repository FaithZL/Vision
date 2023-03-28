//
// Created by Zero on 02/09/2022.
//

#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/context.h"
#include "application.h"

using namespace ocarina;
using namespace vision;

int execute(int argc, char *argv[]) {
    core::log_level_info();
    App app(argc, argv);
    if (argc == 1) {
        app.context.cli_parser().print_help();
        return 0;
    }
    app.prepare();
    return app.run();
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}