//
// Created by Zero on 2024/8/2.
//

#include "file_inspector.h"
#include <windows.h>

namespace vision::inline hotfix {

void FileInspector::add_inspected(const fs::path &path, bool recursive) noexcept {
    string key = path.string();
    if (group_.contains(key) || !fs::exists(path)) {
        return;
    }
    auto is_directory = fs::is_directory(path);
    recursive = is_directory && recursive;

    if (!is_directory) {
        InspectedPath inspected(path);
        group_.insert(std::make_pair(key, inspected));
        return;
    }

    auto func = [&](const fs::directory_entry &entry) {
        if (fs::is_directory(entry)) {
            return ;
        }
        InspectedPath inspected(entry);
        group_.insert(std::make_pair(entry.path().string(), inspected));
    };

    if (recursive) {
        for (const auto &entry : fs::recursive_directory_iterator(path)) {
            func(entry);
        }
    } else {
        for (const auto &entry : fs::directory_iterator(path)) {
            func(entry);
        }
    }
}

void FileInspector::remove_inspected(const fs::path &path, bool recursive) noexcept {
    string key = path.string();
    if (!group_.contains(key)) {
        return;
    }
    if (fs::is_regular_file(path)) {
        group_.erase(key);
    }
    auto func = [&](const fs::directory_entry &entry) {
        group_.erase(entry.path().string());
    };
    if (recursive) {
        for (const auto &entry : fs::recursive_directory_iterator(path)) {
            func(entry);
        }
    } else {
        for (const auto &entry : fs::directory_iterator(path)) {
            func(entry);
        }
    }
}

vector<fs::path> FileInspector::get_updated_files() noexcept {
    vector<fs::path> ret;
    ret.reserve(6);
    auto func = [&](InspectedPath &inspected) {
        FileTime write_time = modification_time(inspected.path);
        if (write_time > inspected.write_time) {
            ret.push_back(inspected.path);
            inspected.write_time = write_time;
        }
    };

    std::for_each(group_.begin(), group_.end(), [&](auto &it) {
        string key = it.first;
        func(it.second);
    });
    return ret;
}

fs::path FileInspector::project_path() noexcept {
    return parent_path(__FILE__, 3);
}

}// namespace vision::inline hotfix