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

using namespace ocarina;
using namespace vision;

int execute(int argc, char *argv[]){

    vision::Context context(argc, argv);
    if (argc == 1) {
        context.cli_parser().print_help();
        return 0;
    }
    context.clear_cache();
    Device device = context.create_device("cuda");
    device.init_rtx();
    SceneDesc scene_desc = context.parse_file();

    RenderPipeline rp = context.create_pipeline(&device);
    rp.init_scene(scene_desc);
    rp.prepare();

    Window::Wrapper window = context.create_window("vision", rp.resolution());

    window->set_key_callback([&](int key, int action) {
        double dt = window->dt();
        Camera *camera = rp.scene().camera();
        float3 forward = camera->forward();
        float3 up = camera->up();
        float3 right = camera->right();
        float distance = camera->velocity() * dt;
        switch (key) {
            case 'W':
                camera->move(forward * distance);
                break;
            case 'A':
                camera->move(-right * distance);
                break;
            case 'S':
                camera->move(-forward * distance);
                break;
            case 'D':
                camera->move(right * distance);
                break;
            case 'Q':
                camera->move(-up * distance);
                break;
            case 'E':
                camera->move(up * distance);
                break;
            default:
                break;
        }
    });

    window->run([&](double dt) {
        rp.update();
        rp.render(dt);
        rp.download_result();
        window->set_background(rp.buffer());
    });

    return 0;
}

int main(int argc, char *argv[]) {
    return execute(argc, argv);
}