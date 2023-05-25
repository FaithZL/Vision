//
// Created by Zero on 02/09/2022.
//

#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/context.h"
#include "application.h"

using namespace ocarina;
using namespace vision;

int main(int argc, char *argv[]) {
    return App(argc, argv).run();
}