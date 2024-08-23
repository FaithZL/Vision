//
// Created by Zero on 2024/8/2.
//

#include "file_inspector.h"
#include <windows.h>

namespace vision::inline hotfix {

void FileInspector::add_inspected(const fs::path &path, string_view module_name, bool recursive) {
    if (target_map_.contains(module_name) || !fs::exists(path)) {
        return;
    }
    auto is_directory = fs::is_directory(path);
    recursive = is_directory && recursive;

    auto add_file = [this](Target &module, const InspectedFile &inspected) {
        if (inspected.path.extension() != ".cpp")
            return;
        module.files.push_back(inspected);
        files_.insert(inspected.path.string());
    };

    Target target;
    target.name = module_name;
    if (!is_directory) {
        InspectedFile inspected(path);
        add_file(target, inspected);
        target_map_.insert(std::make_pair(module_name, target));
        return;
    }

    auto func = [&](const fs::directory_entry &entry) {
        if (entry.exists() && entry.is_regular_file()) {
            auto f = InspectedFile(entry.path());
            ///
            f.write_time = {};
            add_file(target, f);
        }
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
    target_map_.insert(std::make_pair(module_name, target));
}

void FileInspector::remove_inspected(const fs::path &path, bool recursive) noexcept {
    string key = path.string();
    if (!target_map_.contains(key)) {
        return;
    }
    target_map_.erase(key);
}

vector<FileInspector::Target> FileInspector::get_modified_targets() noexcept {
    vector<Target> ret;

    auto is_modified = [&](Target &target) {
        bool modified = false;
        target.modified_files.clear();
        std::for_each(target.files.begin(), target.files.end(), [&](InspectedFile &file) {
            FileTime write_time = modification_time(file.path);
            if (write_time > file.write_time) {
                //                file.write_time = write_time;
                target.modified_files.push_back(file.path);
                modified = true;
            }
        });

        return modified;
    };

    for (auto &it : target_map_) {
        const string_view &key = it.first;
        Target &target = it.second;
        if (is_modified(target)) {
            target.increase_count();
            ret.push_back(target);
        }
        target.modified_files.clear();
    }
    return ret;
}

}// namespace vision::inline hotfix