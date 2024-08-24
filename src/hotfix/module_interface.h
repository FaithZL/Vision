//
// Created by Zero on 2024/7/31.
//

#pragma once

#include "core/stl.h"
#include "core/vs_header.h"
#include "object.h"

namespace vision::inline hotfix {

class ModuleInterface {
private:
    static ModuleInterface *s_module_interface;
    ModuleInterface();
    ~ModuleInterface();

private:
    map<string, const IObjectConstructor*> constructor_map_;

public:
    ModuleInterface(const ModuleInterface &) = delete;
    ModuleInterface(ModuleInterface &&) = delete;
    ModuleInterface operator=(const ModuleInterface &) = delete;
    [[nodiscard]] static string_view src_path() noexcept;
    ModuleInterface operator=(ModuleInterface &&) = delete;
    [[nodiscard]] static ModuleInterface &instance() noexcept;
    void add_constructor(const IObjectConstructor* constructor) noexcept;
};

}// namespace vision::inline hotfix