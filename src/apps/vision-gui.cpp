//
// Created by Zero on 02/09/2022.
//

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "core/context.h"

using namespace ocarina;
using namespace vision;

int execute(int argc, char *argv[]){
    vision::Context context(argc, argv);
    if (argc == 1) {
        context.cli_parser().print_help();
        return 0;
    }
    Device device = context.create_device("cuda");
    device.init_rtx();
    SceneDesc scene_desc = context.parse_file();

    RenderPipeline rp = context.create_pipeline(&device);
    rp.init_scene(scene_desc);
    rp.prepare();


    return 0;
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}