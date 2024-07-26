//
// Created by Zero on 2024/7/29.
//

#pragma once

#include "core/stl.h"
#include "core/hash.h"
#include "serializer.h"
#include "object.h"
#include "build_tool.h"
#include "file_inspector.h"

#define INSPECT_PATH_(path) vision::HotfixSystem::instance().add_inspected(path);

#define REGISTER_FILES(...)                   \
    namespace {                               \
    struct FileRegistrar {                    \
        FileRegistrar() {                     \
            MAP(INSPECT_PATH_, ##__VA_ARGS__) \
        }                                     \
    };                                        \
    static FileRegistrar registrar;           \
    }

#define REGISTER_CURRENT_FILE REGISTER_FILES(__FILE__)

#define REGISTER_PATH(path, level)                           \
    namespace {                                              \
    struct FileRegistrar {                                   \
        FileRegistrar() {                                    \
            INSPECT_PATH_(ocarina::parent_path(path, level)) \
        }                                                    \
    };                                                       \
    static FileRegistrar registrar;                          \
    }

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
    BuildTool build_tool_{};

public:
    HotfixSystem(const HotfixSystem &) = delete;
    HotfixSystem(HotfixSystem &&) = delete;
    HotfixSystem operator=(const HotfixSystem &) = delete;
    HotfixSystem operator=(HotfixSystem &&) = delete;
    void add_object(SP<RuntimeObject> object) noexcept;
    void register_observer(Observer *observer) noexcept;
    void deregister_observer(Observer *observer) noexcept;
    void update(SP<RuntimeObject> object) noexcept {
        update(object->class_name());
    }
    void add_inspected(const fs::path &path, bool recursive = true) noexcept {
        file_inspector_.add_inspected(path, recursive);
    }
    void remove_inspected(const fs::path &path) noexcept {
        file_inspector_.remove_inspected(path);
    }
    void check_and_build() noexcept;
    static void inspect_path(const fs::path &path, int back = 0) noexcept;
    void update(const string &c_name) noexcept;
    void remove_object(SP<RuntimeObject> object) noexcept;
    OC_MAKE_MEMBER_GETTER(serializer, &)
    static HotfixSystem &instance() noexcept;
    static void destroy_instance() noexcept;
};

}// namespace vision::inline hotfix
