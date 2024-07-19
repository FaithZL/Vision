//
// Created by Zero on 2024/8/2.
//

#include "file_observer.h"
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
    return ft_write.dwLowDateTime;
}

}// namespace detail

void FileObserver::add_observed(const fs::path &path, bool recursive) noexcept {
    ObservedFile observed;
    observed.path = path;
    observed.recursive = recursive;
    observed.change_time = detail::get_change_timestamp(path);
    group_.insert(std::make_pair(observed.path.string(), observed));
}

void FileObserver::remove_observed(const fs::path &path) noexcept {
    string key = path.string();
    if (group_.contains(key)) {
        group_.erase(key);
    }
}

void FileObserver::apply() noexcept {

}

}// namespace vision::inline hotfix