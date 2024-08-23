//
// Created by Zero on 2024/8/2.
//

#pragma once

#include "core/stl.h"
#include "core/string_util.h"

namespace vision::inline hotfix {
using namespace ocarina;

static constexpr const char *debug_dir = "hotfix_debug";

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
        explicit InspectedFile(const fs::path &p)
            : path(p), write_time(modification_time(p)) {}
    };

    struct Target {
    private:
        mutable uint build_count{0u};

    public:
        string name;
        vector<InspectedFile> files;
        vector<fs::path> modified_files;
        [[nodiscard]] fs::path temp_directory() const noexcept {
            return FileInspector::intermediate_path() / fs::path(name).stem();
        }
        [[nodiscard]] fs::path target_stem() const noexcept {
            return ocarina::format("module_{}.dll", build_count);
        }
        [[nodiscard]] string target_path(string extension) const noexcept {
            return (temp_directory() / (target_stem().string() + std::move(extension))).string();
        }
        void increase_count() const noexcept { ++build_count; }
        void decrease_count() const noexcept { --build_count; }
    };

    /// key: name, value : Target
    using map_type = map<string_view, Target>;

private:
    map_type target_map_;
    set<string> files_;

public:
    FileInspector() = default;
    void add_inspected(const fs::path &path, string_view module_name, bool recursive = true);
    void remove_inspected(const fs::path &path, bool recursive = true) noexcept;
    [[nodiscard]] vector<Target> get_modified_targets() noexcept;
    [[nodiscard]] bool has_target(string_view target_name) noexcept {
        return target_map_.contains(target_name);
    }
    [[nodiscard]] bool has_file(const string &fn) noexcept {
        return files_.contains(fn);
    }
    [[nodiscard]] static fs::path project_path() noexcept {
        return parent_path(__FILE__, 3);
    }
    [[nodiscard]] static fs::path project_src_path() noexcept {
        return parent_path(__FILE__, 2);
    }
    [[nodiscard]] static fs::path intermediate_path() noexcept {
        return fs::current_path() / debug_dir;
    }
};

}// namespace vision::inline hotfix