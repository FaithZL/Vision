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

class HotfixSystem;

class Observer {
protected:
    bool valid_{true};
    void invalidation() noexcept { valid_ = false; }
    friend class HotfixSystem;

public:
    virtual void update_runtime_object(const IObjectConstructor *constructor) noexcept = 0;
    virtual void notified(const vector<const IObjectConstructor *> &constructors) noexcept {
        for (const auto &item : constructors) {
            update_runtime_object(item);
        }
    }
    Observer();
    ~Observer();
};

class HotfixSystem {
private:
    HotfixSystem() { versions_.push_back({}); }
    ~HotfixSystem();
    static HotfixSystem *s_mgr;

private:
    Serializer serializer_{};
    ocarina::vector<Observer *> observers_;
    ocarina::vector<SP<const Observer>> defer_delete_;
    FileTool file_tool_;
    BuildSystem build_system_{};
    queue<std::function<void()>> callbacks_;
    vector<Version> versions_;
    int cur_ver_{0};

public:
    HotfixSystem(const HotfixSystem &) = delete;
    HotfixSystem(HotfixSystem &&) = delete;
    HotfixSystem operator=(const HotfixSystem &) = delete;
    HotfixSystem operator=(HotfixSystem &&) = delete;
    void register_observer(Observer *observer) noexcept;
    void deregister_observer(Observer *observer) noexcept;
    void defer_delete(SP<const Observer> observer) noexcept;
    OC_MAKE_MEMBER_GETTER(file_tool, &)
    OC_MAKE_MEMBER_GETTER(serializer, &)
    template<typename... Args>
    void register_target(Args &&...args) {
        file_tool_.add_inspected(OC_FORWARD(args)...);
    }
    void remove_inspected(const fs::path &path) noexcept {
        file_tool_.remove_inspected(path);
    }
    void execute_callback();
    [[nodiscard]] bool is_working() const noexcept { return build_system_.is_working(); }
    void enqueue_function(std::function<void()> fn) noexcept;
    void on_build_finish(bool success, const Target &target) noexcept;
    void on_build_all_finish(const vector<pair<bool, Target>> &modules) noexcept;
    void load_module(Target target) noexcept;
    void update_objects(const vector<const IObjectConstructor *> &constructors) noexcept;
    bool check_and_build() noexcept;
    void next_version() noexcept;
    void previous_version() noexcept;
    [[nodiscard]] bool has_previous() const noexcept {
        return cur_ver_ > 0 && !versions_.empty();
    }
    [[nodiscard]] bool has_next() const noexcept {
        return cur_ver_ < versions_.size() - 1 && !versions_.empty();
    }
    template<typename Tuple>
    static void replace_objects(const IObjectConstructor *constructor, Tuple tuple) noexcept {
        traverse_tuple(tuple, [&]<typename T>(SP<T> *ptr) {
            if (constructor->match(ptr->get())) {
                SP<T> new_obj = constructor->construct_shared<T>();
                new_obj->restore(ptr->get());
                if constexpr (std::derived_from<T, Observer>) {
                    instance().defer_delete(*ptr);
                }
                *ptr = new_obj;
            }
        });
    }
    void on_version_change() noexcept;
    static HotfixSystem &instance() noexcept;
    static void destroy_instance() noexcept;
};

}// namespace vision::inline hotfix
