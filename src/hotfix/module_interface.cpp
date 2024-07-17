//
// Created by Zero on 2024/7/31.
//

#include "module_interface.h"

namespace vision::inline hotfix {

ModuleInterface *ModuleInterface::s_module_interface = nullptr;

ModuleInterface &ModuleInterface::instance() noexcept {
    if (s_module_interface) {
        s_module_interface = new ModuleInterface();
    }
    return *s_module_interface;
}

ModuleInterface::~ModuleInterface() {}

}// namespace vision::inline hotfix

extern "C" __declspec(dllexport) vision::ModuleInterface *module_interface() {
    return &vision::ModuleInterface::instance();
}
