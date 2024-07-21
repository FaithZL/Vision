//
// Created by Zero on 2024/8/2.
//

#pragma once

#include "core/stl.h"

namespace vision::inline hotfix {
using namespace ocarina;
class FileInspector {
public:
    enum Action {
        Add = 1,
        Delete = 1 << 1,
        Modify = 1 << 2,
    };

    struct InspectedPath {
    public:
        ocarina::fs::path path{};
        bool recursive{};
        Action action{Modify};
        uint32_t write_time{};
    };

    using group_type = map<string, InspectedPath>;

private:
    group_type group_;

public:
    FileInspector() = default;
    void add_inspected(const fs::path &path, bool recursive = true) noexcept;
    void remove_inspected(const fs::path &path) noexcept;
    void apply() noexcept;
};

}// namespace vision::inline hotfix