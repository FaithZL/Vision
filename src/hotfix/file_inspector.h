//
// Created by Zero on 2024/8/2.
//

#pragma once

#include "core/stl.h"

namespace vision::inline hotfix {
using namespace ocarina;

inline uint32_t get_change_timestamp(const fs::path &path) {
    FILETIME ft_create, ft_access, ft_write;
    auto fn = path.string();
    HANDLE file = CreateFile(fn.c_str(), GENERIC_READ, FILE_SHARE_READ,
                             nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
    GetFileTime(file, &ft_create, &ft_access, &ft_write);
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft_write, &st);
    return ft_write.dwLowDateTime;
}

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

    struct InspectedPath {
    public:
        ocarina::fs::path path{};
        Action action{Modify};
        FileTime write_time{};
        InspectedPath() = default;
        InspectedPath(const fs::path &p)
            : path(p), write_time(modification_time(p)) {}
    };

    using group_type = map<string, vector<InspectedPath>>;

private:
    group_type group_;

public:
    FileInspector() = default;
    void add_inspected(const fs::path &path, bool recursive = true) noexcept;
    void remove_inspected(const fs::path &path) noexcept;
    [[nodiscard]] vector<fs::path> get_updated_files() noexcept;
};

}// namespace vision::inline hotfix