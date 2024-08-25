//
// Created by Zero on 2024/8/21.
//

#include "demo.h"
#include "core/vs_header.h"
#include "hotfix/hotfix.h"
#include "hotfix/module_interface.h"

namespace vision::inline hotfix {

void Demo::serialize(vision::Serializer *serializer) const noexcept {
    std::cout << "wocao222" << endl;
}

void Demo::deserialize(vision::Serializer *serializer) const noexcept {
}

}// namespace vision::inline hotfix

namespace {
struct ConstructorRegistrar {
    ConstructorRegistrar() {
        using namespace vision::hotfix;
        ModuleInterface::instance().add_constructor(ocarina::make_shared<ObjectConstructor<Demo>>());
    }
};
static ConstructorRegistrar registrar;
}// namespace
