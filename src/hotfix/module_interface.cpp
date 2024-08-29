//
// Created by Zero on 2024/7/31.
//

#include "module_interface.h"
#include "core/vs_header.h"
#include "object.h"

namespace vision::inline hotfix {

ModuleInterface::ModuleInterface() = default;

ModuleInterface &ModuleInterface::instance() noexcept {
    static ModuleInterface s_module_interface;
    return s_module_interface;
}

void ModuleInterface::add_constructor(UP<const IObjectConstructor> constructor) noexcept {
    string_view name = constructor->class_name();
    constructor_map_.insert(make_pair(name, ocarina::move(constructor)));
}

const IObjectConstructor *ModuleInterface::constructor(const std::string &cls_name) const noexcept {
    OC_ASSERT(constructor_map_.contains(cls_name));
    return constructor_map_.at(cls_name).get();
}

vector<const IObjectConstructor *> ModuleInterface::constructors(const std::string &filename) const noexcept {
    vector<const IObjectConstructor *> ret;
    for (const auto &item : constructor_map_) {
        const IObjectConstructor *ptr = item.second.get();
        if (ptr->filename() == filename) {
            ret.push_back(ptr);
        }
    }
    return ret;
}

string_view ModuleInterface::src_path() noexcept {
    return __FILE__;
}

ModuleInterface::~ModuleInterface() {
    cout << "ModuleInterface  exit" <<endl;
}

}// namespace vision::inline hotfix

VS_EXPORT_API vision::ModuleInterface *module_interface() {
    return &vision::ModuleInterface::instance();
}


