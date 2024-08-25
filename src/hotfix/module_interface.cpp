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

void ModuleInterface::add_constructor(SP<const IObjectConstructor> constructor) noexcept {
    string_view name = constructor->class_name();
    constructor_map_.insert(make_pair(name, ocarina::move(constructor)));
}

const IObjectConstructor *ModuleInterface::constructor(const std::string &cls_name) const noexcept {
    return constructor_map_.at(cls_name).get();
}

string_view ModuleInterface::src_path() noexcept {
    return __FILE__;
}

ModuleInterface::~ModuleInterface() = default;

}// namespace vision::inline hotfix

VS_EXPORT_API vision::ModuleInterface *module_interface() {
    return &vision::ModuleInterface::instance();
}


