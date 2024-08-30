//
// Created by Zero on 2024/7/29.
//

#include "hotfix.h"
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

void HotfixSystem::on_build_finish(const vector<Target> &targets, bool success) noexcept {
    
    OC_INFO("build finish ");

    vector<const IObjectConstructor *> constructors;

    auto process_target = [&](const Target &target) {
        ModuleInterface *module_interface = target.module_interface();
        auto tmp = module_interface->constructors(target.modified_files);
        constructors.insert(constructors.cend(), tmp.cbegin(), tmp.cend());
    };

    std::for_each(targets.begin(), targets.end(), process_target);

    for (Observer *item : observers_) {
        item->notified(constructors);
    }
}

void HotfixSystem::check_and_build() noexcept {
    auto modules = file_tool_.get_modified_targets();
    if (modules.empty()) {
        return;
    }
    build_system_.build_targets(modules, [this,modules](const string &cmd, bool success) {
        this->on_build_finish(modules, success);
    });
}

void HotfixSystem::init() noexcept {
    build_system_.init();
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

HotfixSystem::~HotfixSystem() = default;

}// namespace vision::inline hotfix

//VS_REGISTER_CURRENT_PATH(1, "vision-hotfix.dll", false)