//
// Created by Zero on 2024/7/31.
//

#include "module_interface.h"
#include "core/vs_header.h"
#include "object.h"

namespace vision::inline hotfix {

ModuleInterface *ModuleInterface::s_module_interface = nullptr;

ModuleInterface::ModuleInterface() = default;

ModuleInterface &ModuleInterface::instance() noexcept {
    if (!s_module_interface) {
        s_module_interface = new ModuleInterface();
    }
    return *s_module_interface;
}

void ModuleInterface::add_constructor(const vision::IObjectConstructor *constructor) noexcept {
    constructor_map_.insert(make_pair(constructor->class_name(), constructor));
}

string_view ModuleInterface::src_path() noexcept {
    return __FILE__;
}

ModuleInterface::~ModuleInterface() = default;

}// namespace vision::inline hotfix

VS_EXPORT_API vision::ModuleInterface *module_interface() {
    return &vision::ModuleInterface::instance();
}


