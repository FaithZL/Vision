//
// Created by Zero on 2024/7/31.
//

#include "hotfix/hotfix.h"
#include "core/stl.h"
#include "dsl/dsl.h"
#include "util/file_manager.h"
#include "rhi/common.h"
#include <windows.h>

#include <memory>
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

    float3x3 f333;
    auto f2 = float2(1,2);
    auto f22 = float2(8,9);
    auto f222 = float2(6,7);
    auto f3 = float3(2,5,7);
    Matrix<2,2> m(f2, f222);
    Matrix<2,2> m1{f22,f2};

    float2x2 fm(f2, f222);
    float2x2 fm1{f22,f2};

    auto m3 = m1 - m;
    auto fm3 = fm1 - fm;

    auto m4 = -m3;

    vision::HotfixSystem::instance().init();

    auto window = FileManager::instance().create_window("display", make_uint2(500), "imGui");
    auto image_io = Image::pure_color(make_float4(1, 0, 0, 1), ColorSpace::LINEAR, make_uint2(500));
    window->init_widgets();

    auto widget = window->widgets();
    using fun_t = vision::Demo *();
    using fun2_t = vision::hotfix::ModuleInterface *();

    auto &mi = vision::ModuleInterface::instance();

    vision::Serializer serializer;

    auto demo = std::make_shared<vision::Demo>();
    vision::Test *test = new vision::Test();

    window->run([&](double d) {
        widget->button_click("hotfix", [&] {
            vision::HotfixSystem::instance().check_and_build();
        });

        widget->button_click("reload", [&] {
            auto &inspector = vision::HotfixSystem::instance().file_tool();

            auto target = inspector.get_target("vision-hotfix-test.dll");

            auto module = FileManager::instance().obtain_module(target.dll_path().string());

            auto func2 = module->function<fun2_t *>("module_interface");
            auto *mi = func2();
            auto constructor = mi->constructor(vision::Demo().class_name());

            demo = constructor->construct_shared<vision::Demo>();
            test = mi->construct<vision::Test>();
        });

        widget->button_click("clear", [&] {
            demo->clear();
            test->clear();
        });

        widget->button_click("fill", [&] {
            demo->fill();
            test->fill();
        });

        widget->button_click("serialize", [&] {
            serializer.serialize(demo);
            serializer.serialize(test);
        });

        widget->button_click("deserialize", [&] {
            serializer.deserialize(demo.get(), demo);
            serializer.deserialize(test, test);
        });

        widget->button_click("print object", [&] {
            demo->print();
            test->print();
        });

        widget->button_click("print serializer", [&] {
            serializer.print();
        });
    });

    return 0;
}