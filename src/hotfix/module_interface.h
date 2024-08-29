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
    ModuleInterface();
    ~ModuleInterface();

private:
    map<string, UP<const IObjectConstructor>> constructor_map_;

public:
    ModuleInterface(const ModuleInterface &) = delete;
    ModuleInterface(ModuleInterface &&) = delete;
    ModuleInterface operator=(const ModuleInterface &) = delete;
    [[nodiscard]] static string_view src_path() noexcept;
    ModuleInterface operator=(ModuleInterface &&) = delete;
    [[nodiscard]] static ModuleInterface &instance() noexcept;
    void add_constructor(UP<const IObjectConstructor> constructor) noexcept;
    [[nodiscard]] const IObjectConstructor* constructor(const string &cls_name) const noexcept;
    template<typename T>
    [[nodiscard]] T *construct() const noexcept {
        return constructor(type_string<T>())->template construct<T>();
    }
    template<typename T>
    [[nodiscard]] SP<T> construct_shared() const noexcept {
        return constructor(type_string<T>())->template construct_shared<T>();
    }
    template<typename T>
    [[nodiscard]] UP<T> construct_unique() const noexcept {
        return constructor(type_string<T>())->template construct_unique<T>();
    }
};

}// namespace vision::inline hotfix