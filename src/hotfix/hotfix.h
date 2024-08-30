//
// Created by Zero on 2024/7/29.
//

#pragma once

#include "macro.h"
#include "core/hash.h"
#include "serializer.h"
#include "object.h"
#include "build_system.h"
#include "file_tool.h"

namespace vision::inline hotfix {

using namespace ocarina;

class Observer {
public:
    virtual void update_runtime_object(const IObjectConstructor *constructor) noexcept = 0;
    virtual void notified(const vector<const IObjectConstructor*> &constructors) noexcept {
        for (const auto &item : constructors) {
            update_runtime_object(item);
        }
    }
    Observer();
    ~Observer();
};

class HotfixSystem {
private:
    HotfixSystem() = default;
    ~HotfixSystem();
    static HotfixSystem *s_mgr;

private:
    using ObjectGroup = vector<SP<RuntimeObject>>;
    Serializer serializer_{};
    ocarina::set<Observer *> observers_;
    FileTool file_tool_;
    BuildSystem build_system_{};

public:
    HotfixSystem(const HotfixSystem &) = delete;
    HotfixSystem(HotfixSystem &&) = delete;
    HotfixSystem operator=(const HotfixSystem &) = delete;
    HotfixSystem operator=(HotfixSystem &&) = delete;
    void register_observer(Observer *observer) noexcept;
    void deregister_observer(Observer *observer) noexcept;
    void init() noexcept;
    OC_MAKE_MEMBER_GETTER(file_tool, &)
    OC_MAKE_MEMBER_GETTER(serializer, &)
    template<typename... Args>
    void register_target(Args &&...args) {
        file_tool_.add_inspected(OC_FORWARD(args)...);
    }
    void remove_inspected(const fs::path &path) noexcept {
        file_tool_.remove_inspected(path);
    }
    void on_build_finish(const vector<Target> &targets) noexcept;
    void check_and_build() noexcept;
    static HotfixSystem &instance() noexcept;
    static void destroy_instance() noexcept;
};

}// namespace vision::inline hotfix
