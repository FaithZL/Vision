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
    ModuleInterface() = default;
    ~ModuleInterface();

public:
    ModuleInterface(const ModuleInterface &) = delete;
    ModuleInterface(ModuleInterface &&) = delete;
    ModuleInterface operator=(const ModuleInterface &) = delete;
    ModuleInterface operator=(ModuleInterface &&) = delete;
    static ModuleInterface &instance() noexcept;
};

}// namespace vision::inline hotfix