//
// Created by Zero on 02/09/2022.
//

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "core/context.h"
#include "util/image_io.h"

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
    rp.build_accel();

    auto window = context.create_window("vision", rp.resolution());
    auto image_io = ImageIO::pure_color(make_float4(1,0,0,1), ColorSpace::LINEAR, rp.resolution());
    window->run([&](double dt) {
        rp.render(dt);
        window->set_background(rp.buffer());
    });

    return 0;
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}