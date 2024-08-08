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

    using FileTime = std::chrono::time_point<std::chrono::file_clock>;

    static FileTime modification_time(const fs::path &file_path) {
        if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
            throw std::runtime_error("File does not exist or is not a regular file.");
        }
        FileTime f_time = fs::last_write_time(file_path);
        return f_time;
    }

    struct InspectedFile {
    public:
        fs::path path{};
        Action action{Modify};
        FileTime write_time{};
        InspectedFile() = default;
        InspectedFile(const fs::path &p)
            : path(p), write_time(modification_time(p)) {}
    };

    using map_type = map<string, InspectedFile>;

    struct Module {
        vector<InspectedFile> files;
        vector<fs::path> modified_files;
        string name;
    };

    /// key: path(directory or file), value : Module
    using groups_type = map<string, Module>;

private:
    groups_type groups_;

public:
    FileInspector() = default;
    void add_inspected(const fs::path &path, bool recursive = true) noexcept;
    void remove_inspected(const fs::path &path, bool recursive = true) noexcept;
    [[nodiscard]] vector<Module> get_modified_modules() noexcept;
    [[nodiscard]] static fs::path project_path() noexcept;
};

}// namespace vision::inline hotfix