//
// Created by Zero on 2024/7/29.
//

#pragma once

#include "core/stl.h"
#include "core/hash.h"
#include "serializer.h"
#include "object.h"
#include "build_system.h"
#include "file_inspector.h"

#define VS_REGISTER_PATH(path, level, ...)                                        \
    namespace {                                                                   \
    struct FileRegistrar {                                                        \
        FileRegistrar() {                                                         \
            auto key = ocarina::parent_path(path, level);                         \
            vision::HotfixSystem::instance().register_target(key, ##__VA_ARGS__); \
        }                                                                         \
    };                                                                            \
    static FileRegistrar registrar;                                               \
    }

#define VS_REGISTER_CURRENT_PATH(level, ...) VS_REGISTER_PATH(__FILE__, level, ##__VA_ARGS__)

namespace vision::inline hotfix {

using namespace ocarina;

class Observer {
public:
    virtual void on_update(RuntimeObject *old_obj,
                           RuntimeObject *new_obj) noexcept = 0;
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
    map<string, ObjectGroup> map_;
    Serializer serializer_{};
    ocarina::set<Observer *> observers_;
    FileInspector file_inspector_;
    BuildSystem build_system_{};

public:
    HotfixSystem(const HotfixSystem &) = delete;
    HotfixSystem(HotfixSystem &&) = delete;
    HotfixSystem operator=(const HotfixSystem &) = delete;
    HotfixSystem operator=(HotfixSystem &&) = delete;
    void add_object(SP<RuntimeObject> object) noexcept;
    void register_observer(Observer *observer) noexcept;
    void deregister_observer(Observer *observer) noexcept;
    void init() noexcept;
    void update(SP<RuntimeObject> object) noexcept {
        update(object->class_name());
    }
    template<typename ...Args>
    void register_target(Args &&...args) {
        file_inspector_.add_inspected(OC_FORWARD(args)...);
    }
    void remove_inspected(const fs::path &path) noexcept {
        file_inspector_.remove_inspected(path);
    }
    void check_and_build() noexcept;
    void update(const string &c_name) noexcept;
    void remove_object(SP<RuntimeObject> object) noexcept;
    OC_MAKE_MEMBER_GETTER(serializer, &)
    static HotfixSystem &instance() noexcept;
    static void destroy_instance() noexcept;
};

}// namespace vision::inline hotfix
