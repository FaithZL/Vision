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
#include "hotfix/module_interface.h"

using namespace ocarina;

int main(int argc, char *argv[]) {
    fs::path path(argv[0]);

    vision::Test test;

    vision::HotfixSystem::instance().init();

    auto window = FileManager::instance().create_window("display", make_uint2(500), "imGui");
    auto image_io = Image::pure_color(make_float4(1, 0, 0, 1), ColorSpace::LINEAR, make_uint2(500));
    window->init_widgets();

    auto widget = window->widgets();
    using fun_t = vision::Demo *();
    using fun2_t = vision::hotfix::ModuleInterface *();

    auto &mi = vision::ModuleInterface::instance();

    vision::Serializer serializer;

    window->run([&](double d) {
        widget->button_click("hotfix", [&] {
            vision::HotfixSystem::instance().check_and_build();
        });

        widget->button_click("test", [&] {
            auto &inspector = vision::HotfixSystem::instance().file_tool();

            auto target = inspector.get_target("vision-hotfix-test.dll");

            auto module = FileManager::instance().obtain_module(target.dll_path().string());

            auto func2 = module->function<fun2_t *>("module_interface");
            auto *mi = func2();
            auto constructor = mi->constructor(vision::Demo().class_name());

            auto *dd = constructor->construct();
            dd->serialize(addressof(serializer));

            auto *d2 = mi->constructor(vision::Test().class_name())->construct();
            d2->serialize(addressof(serializer));

            int i = 0;
        });
    });

    return 0;
}