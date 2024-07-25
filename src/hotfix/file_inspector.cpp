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
    vector<InspectedPath> paths;

    if (!is_directory) {
        InspectedPath inspected(path);
        paths.push_back(inspected);
        group_.insert(std::make_pair(key, paths));
        return;
    }

    auto func = [&](auto entry) {
        if (fs::is_directory(entry)) {
            return ;
        }
        InspectedPath inspected(entry);
        paths.push_back(inspected);
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
    group_.insert(std::make_pair(key, paths));
}

void FileInspector::remove_inspected(const fs::path &path) noexcept {
    string key = path.string();
    if (group_.contains(key)) {
        group_.erase(key);
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
        vector<InspectedPath> &paths = it.second;
        std::for_each(paths.begin(), paths.end(), func);
    });
    return ret;
}

}// namespace vision::inline hotfix