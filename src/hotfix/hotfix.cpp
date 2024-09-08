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

void HotfixSystem::defer_delete(SP<const Observer> observer) noexcept {
    const_cast<Observer *>(observer.get())->invalidation();
    defer_delete_.push_back(ocarina::move(observer));
}

void HotfixSystem::enqueue_function(std::function<void()> fn) noexcept {
    callbacks_.push(ocarina::move(fn));
}

void HotfixSystem::execute_callback() {
    while (!callbacks_.empty()) {
        std::invoke(callbacks_.front());
        callbacks_.pop();
    }
}

void HotfixSystem::load_module(vision::Target target) noexcept {
    vector<const IObjectConstructor *> constructors;
    ModuleInterface *module_interface = target.module_interface();
    auto tmp = module_interface->constructors(target.modified_files);
    constructors.insert(constructors.cend(), tmp.cbegin(), tmp.cend());
    for (int i = 0; i < observers_.size(); ++i) {
        Observer *observer = observers_[i];
        if (!observer->valid_) {
            continue;
        }
        observer->notified(constructors);
    }
    defer_delete_.clear();
}

void HotfixSystem::on_build_finish(bool success, const Target &target) noexcept {

    if (!success) {
        OC_INFO_FORMAT("module {} build failure!", target.name);
        return;
    }
    OC_INFO_FORMAT("module {} build success!", target.name);

    load_module(target);
}

void HotfixSystem::on_version_change() noexcept {
    if (versions_.empty()) {
        return;
    }
    Version version = versions_[cur_ver_];
    for (const auto &item : version.targets) {
        load_module(item);
    }
}

void HotfixSystem::previous_version() noexcept {
    cur_ver_ = ocarina::clamp(cur_ver_ - 1, 0, int(versions_.size() - 1));
    on_version_change();
}

void HotfixSystem::next_version() noexcept {
    cur_ver_ = ocarina::clamp(cur_ver_ + 1, 0, int(versions_.size() - 1));
    on_version_change();
}

bool HotfixSystem::check_and_build() noexcept {
    auto modules = file_tool_.get_modified_targets();
    if (modules.empty()) {
        return false;
    }
    build_system_.build_targets(modules, [this]<typename... Args>(Args &&...args) {
        this->on_build_finish(OC_FORWARD(args)...);
    });
    Version version;
    version.targets = ocarina::move(modules);
    versions_.push_back(version);
    cur_ver_ = versions_.size() - 1;
    return true;
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