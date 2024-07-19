//
// Created by Zero on 2024/8/2.
//

#pragma once

#include "core/stl.h"

namespace vision::inline hotfix {
using namespace ocarina;
class FileObserver {
public:
    enum Action {
        Add = 1,
        Delete = 1 << 1,
        Modify = 1 << 2,
    };

    struct ObservedFile {
    public:
        ocarina::fs::path path{};
        bool recursive{};
        Action action{Modify};
        uint32_t change_time{};
    };

    using group_type = map<string, ObservedFile>;

private:
    group_type group_;

public:
    FileObserver() = default;
    void add_observed(const fs::path &path, bool recursive = false) noexcept;
    void remove_observed(const fs::path &path) noexcept;
    void apply() noexcept;
};

}// namespace vision::inline hotfix