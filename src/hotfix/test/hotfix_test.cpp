//
// Created by Zero on 2024/8/30.
//

#include "hotfix_test.h"
#include "core/vs_header.h"
#include "hotfix/hotfix.h"
#include "test.h"
#include "hotfix/module_interface.h"

namespace vision::inline hotfix {
using namespace ocarina;
void HotfixTest::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    if (constructor->match(test)) {
        auto new_obj = constructor->construct_shared<Test>();
        auto serialized_data = test->serialized_data();
        new_obj->deserialize(serialized_data);
        HotfixSystem::instance().serializer().erase_old_object(test.get());
        test = new_obj;
    } else if (constructor->match(demo)) {
        auto new_obj = constructor->construct_shared<Demo>();
        auto serialized_data = demo->serialized_data();
        new_obj->deserialize(serialized_data);
        HotfixSystem::instance().serializer().erase_old_object(demo.get());
        demo = new_obj;
    }
}
}// namespace vision::inline hotfix