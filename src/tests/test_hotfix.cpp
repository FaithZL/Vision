//
// Created by Zero on 2024/7/31.
//

#include "hotfix/hotfix.h"
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
#include "hotfix/test/test.h"
#include "hotfix/test/demo.h"

using namespace ocarina;

int main(int argc, char *argv[]) {
    fs::path path(argv[0]);

    vision::Test test;

    vision::HotfixSystem::instance().init();


    auto window = FileManager::instance().create_window("display", make_uint2(500), "imGui");
    auto image_io = Image::pure_color(make_float4(1, 0, 0, 1), ColorSpace::LINEAR, make_uint2(500));
    window->init_widgets();

    auto widget = window->widgets();


    window->run([&](double d) {
        widget->button_click("hotfix", [&] {
            vision::HotfixSystem::instance().check_and_build();
        });

        widget->button_click("test", [&] {
            auto module = FileManager::instance().obtain_module("vision-hotfix-test333.dll");
            using fun_t = vision::Demo *();
            auto func = module->function<fun_t*>("create");

            vision::Demo * dd = func();

            dd->constructor();

            FileManager::instance().unload_module("vision-hotfix-test333.dll");

        });

    });

    return 0;
}