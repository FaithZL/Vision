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
    if (std::find(observers_.cbegin(), observers_.cend(), observer) != observers_.cend()) {
        return;
    }
    observers_.push_back(observer);
}

void HotfixSystem::deregister_observer(vision::Observer *observer) noexcept {
    erase_if(observers_, [&](const Observer *iter) -> bool {
        return observer == iter;
    });
}

void HotfixSystem::on_build_finish(bool success, const Target &target) noexcept {

    if (!success) {
        OC_INFO_FORMAT("module {} build failure!", target.name);
        return;
    }
    OC_INFO_FORMAT("module {} build success!", target.name);

    vector<const IObjectConstructor *> constructors;
    ModuleInterface *module_interface = target.module_interface();
    auto tmp = module_interface->constructors(target.modified_files);
    ModuleInterface::instance().update_constructors(module_interface);
    constructors.insert(constructors.cend(), tmp.cbegin(), tmp.cend());
    for (Observer *item : observers_) {
        item->notified(constructors);
    }
}

void HotfixSystem::check_and_build() noexcept {
    auto modules = file_tool_.get_modified_targets();
    if (modules.empty()) {
        return;
    }
    build_system_.build_targets(modules, [this]<typename... Args>(Args &&...args) {
        this->on_build_finish(OC_FORWARD(args)...);
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