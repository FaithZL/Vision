//
// Created by Zero on 2024/8/2.
//

#include "file_observer.h"

namespace vision::inline hotfix {

void FileObserver::add_observed(const fs::path &path, bool recursive) noexcept {
    ObservedFile observed;
    observed.path = path;
    observed.recursive = recursive;
    group_.insert(std::make_pair(observed.path.string(), observed));
}

void FileObserver::remove_observed(const fs::path &path) noexcept {
    string key = path.string();
    if (group_.contains(key)) {
        group_.erase(key);
    }
}

}// namespace vision::inline hotfix