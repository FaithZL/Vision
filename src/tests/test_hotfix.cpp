//
// Created by Zero on 2024/7/31.
//

#include "hotfix/system.h"
#include "core/stl.h"
#include "dsl/dsl.h"
#include "util/file_manager.h"
#include "rhi/common.h"
#include <windows.h>
#include "math/base.h"
#include "util/image.h"
#include "dsl/dsl.h"
#include "GUI_impl/imGui/window.h"
#include "util/image.h"

using namespace ocarina;

int main(int argc, char *argv[]) {

    fs::path path(argv[0]);
    FileManager file_manager(path.parent_path());

    auto window = file_manager.create_window("display", make_uint2(500), "imGui");
    auto image_io = Image::pure_color(make_float4(1, 0, 0, 1), ColorSpace::LINEAR, make_uint2(500));
    window->init_widgets();

    auto widget = window->widgets();


    auto pt = ocarina::parent_path(__FILE__, 2) / "render_core";

    for (const auto& entry : fs::recursive_directory_iterator(pt)) {
        if (fs::is_regular_file(entry)) {
            std::cout << "File: " << entry.path() << std::endl;
        } else if (fs::is_directory(entry)) {
//            std::cout << "Directory: " << entry.path() << std::endl;
        }
    }

    vision::HotfixSystem::inspect_path(__FILE__, 1);

    window->run([&](double d) {
        widget->button_click("hotfix", [&] {
            OC_INFO("SAFASDFAS");
        });

    });

    return 0;
}