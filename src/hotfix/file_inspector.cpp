//
// Created by Zero on 2024/8/2.
//

#include "file_inspector.h"
#include <windows.h>

namespace vision::inline hotfix {

namespace detail {

uint32_t get_change_timestamp(const fs::path &path) {
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

}// namespace detail

void FileInspector::add_inspected(const fs::path &path, bool recursive) noexcept {
    string key = path.string();
    if (group_.contains(key)) {
        return;
    }
    InspectedPath inspected_path;
    inspected_path.path = path;
    inspected_path.recursive = recursive;
    inspected_path.write_time = detail::get_change_timestamp(path);
    inspected_path.action = Action::Modify;
    group_.insert(std::make_pair(inspected_path.path.string(), inspected_path));
}

void FileInspector::remove_inspected(const fs::path &path) noexcept {
    string key = path.string();
    if (group_.contains(key)) {
        group_.erase(key);
    }
}

void FileInspector::apply() noexcept {
    std::for_each(group_.begin(), group_.end(), [&](auto it) {
        string key = it.first;
        InspectedPath inspected = it.second;
        auto write_ft = detail::get_change_timestamp(inspected.path);
        if (write_ft > inspected.write_time) {

        }
    });
}

}// namespace vision::inline hotfix