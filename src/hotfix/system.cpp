//
// Created by Zero on 2024/7/29.
//

#include "system.h"
#include "core/logging.h"

namespace vision ::inline hotfix {

HotfixSystem *HotfixSystem::s_mgr = nullptr;

Observer::Observer() {
    HotfixSystem::instance().register_observer(this);
}

Observer::~Observer() {
    HotfixSystem::instance().deregister_observer(this);
}

void HotfixSystem::register_observer(vision::Observer *observer) noexcept {
    observers_.insert(observer);
}

void HotfixSystem::deregister_observer(vision::Observer *observer) noexcept {
    observers_.erase(observer);
}

void HotfixSystem::add_object(SP<vision::RuntimeObject> object) noexcept {
    string c_name = object->class_name();
    if (!map_.contains(c_name)) {
        map_.insert(make_pair(c_name, ObjectGroup{}));
    }
    map_[c_name].push_back(std::move(object));
}

void HotfixSystem::remove_object(SP<vision::RuntimeObject> object) noexcept {
    string c_name = object->class_name();
    if (!map_.contains(c_name)) {
        return;
    }
    ObjectGroup &group = map_[c_name];
    auto iter = std::find_if(group.begin(), group.end(), [&](const SP<RuntimeObject>& element) {
        return element.get() == object.get();
    });
    if (iter == group.end()) {
        return;
    }
    group.erase(iter);
}

void HotfixSystem::check_and_build() noexcept {
    auto modules = file_inspector_.get_modified_modules();
    if(modules.empty()) {
        return;
    }
    build_tool_.build_modules(modules);
}

void HotfixSystem::inspect_path(const fs::path &path, int back) noexcept {
    instance().add_inspected(ocarina::parent_path(path, back), true);
}

void HotfixSystem::update(const std::string &c_name) noexcept {
    if (!map_.contains(c_name)) {
        return;
    }
    ObjectGroup &group = map_[c_name];
    std::for_each(observers_.begin(), observers_.end(), [&](Observer *observer) {

    });
}

HotfixSystem &HotfixSystem::instance() noexcept {
    if (s_mgr == nullptr) {
        s_mgr = new HotfixSystem();
    }
    return *s_mgr;
}

void HotfixSystem::destroy_instance() noexcept {
    if (s_mgr == nullptr) {
        return;
    }
    delete s_mgr;
    s_mgr = nullptr;
}

HotfixSystem::~HotfixSystem() {}

}// namespace vision::inline hotfix