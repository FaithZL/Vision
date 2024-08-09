//
// Created by Zero on 2024/8/2.
//

#include "file_inspector.h"
#include <windows.h>

namespace vision::inline hotfix {

void FileInspector::add_inspected(const fs::path &path, bool recursive) {
    string key = path.string();
    if (module_map_.contains(key) || !fs::exists(path)) {
        return;
    }
    auto is_directory = fs::is_directory(path);
    recursive = is_directory && recursive;

    Module module;
    module.name = path.stem().string();
    if (!is_directory) {
        InspectedFile inspected(path);
        module.files.push_back(inspected);
        module_map_.insert(std::make_pair(key, module));
        return;
    }

    auto func = [&](const fs::directory_entry &entry) {
        if (entry.exists() && entry.is_regular_file()) {
            auto f = InspectedFile(entry.path());
            module.files.push_back(f);
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
    module_map_.insert(std::make_pair(key, module));
}

void FileInspector::remove_inspected(const fs::path &path, bool recursive) noexcept {
    string key = path.string();
    if (!module_map_.contains(key)) {
        return;
    }
    module_map_.erase(key);
}

FileInspector::Module &FileInspector::get_module(const fs::path &key) noexcept {
    return module_map_.at(key.string());
}

vector<FileInspector::Module> FileInspector::get_modified_modules() noexcept {
    vector<Module> ret;

    auto is_modified = [&](Module &module) {
        bool modified = false;
        module.modified_files.clear();
        std::for_each(module.files.begin(), module.files.end(), [&](InspectedFile &file) {
            FileTime write_time = modification_time(file.path);
            if (write_time > file.write_time) {
                file.write_time = write_time;
                module.modified_files.push_back(file.path);
                modified = true;
            }
        });

        return modified;
    };

    for (auto &it : module_map_) {
        const string &key = it.first;
        Module &module = it.second;
        if (is_modified(module)) {
            ret.push_back(module);
        }
        module.modified_files.clear();
    }
    return ret;
}

fs::path FileInspector::intermediate_path() noexcept {
    return fs::current_path() / debug_dir;
}

fs::path FileInspector::project_path() noexcept {
    return parent_path(__FILE__, 3);
}

fs::path FileInspector::project_src_path() noexcept {
    return parent_path(__FILE__, 2);
}

}// namespace vision::inline hotfix