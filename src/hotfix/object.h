//
// Created by Zero on 2024/7/31.
//

#pragma once

#include "core/hash.h"
#include "macro.h"

namespace vision::inline hotfix {
using namespace ocarina;

class Serializer;

class ISerialized;

#define VS_HOTFIX_MOVE_ATTR(attr_name) \
    attr_name = ocarina::move(old_obj_->attr_name);

#define VS_HOTFIX_MOVE_ATTRS(Type, ...)            \
    auto old_obj_ = dynamic_cast<Type *>(old_obj); \
    MAP(VS_HOTFIX_MOVE_ATTR, ##__VA_ARGS__)

#define VS_HOTFIX_MAKE_RESTORE(Super, Type, ...)             \
    void restore(RuntimeObject *old_obj) noexcept override { \
        Super::restore(old_obj);                             \
        VS_HOTFIX_MOVE_ATTRS(Type, ##__VA_ARGS__)            \
    }

class RuntimeObject : public Hashable {
public:
    RuntimeObject() = default;
    [[nodiscard]] SP<ISerialized> serialized_data() const noexcept;
    virtual void restore(RuntimeObject *old_obj) noexcept;
    virtual void serialize(SP<ISerialized> output) const noexcept {}
    virtual void deserialize(SP<ISerialized> input) noexcept {}
    virtual ~RuntimeObject() = default;
};

class IObjectConstructor {
protected:
    string_view filename_{};

public:
    explicit IObjectConstructor(const char *fn) : filename_(fn) {}
    OC_MAKE_MEMBER_GETTER(filename, )
    template<typename T>
    [[nodiscard]] bool match(T &&t) const noexcept {
        return class_name() == t->class_name();
    }
    template<typename T = RuntimeObject>
    [[nodiscard]] T *construct() const {
        return dynamic_cast<T *>(construct_impl());
    }
    template<typename T = RuntimeObject>
    [[nodiscard]] SP<T> construct_shared() const noexcept {
        return SP<T>(construct<T>());
    }
    template<typename T = RuntimeObject>
    [[nodiscard]] UP<T> construct_unique() const noexcept {
        return UP<T>(construct<T>());
    }
    [[nodiscard]] virtual RuntimeObject *construct_impl() const = 0;
    static void destroy(RuntimeObject *obj) {
        delete obj;
    }
    [[nodiscard]] virtual string_view class_name() const = 0;
    virtual ~IObjectConstructor() = default;
};

template<typename T>
requires std::derived_from<T, RuntimeObject>
class ObjectConstructor : public IObjectConstructor {
public:
    explicit ObjectConstructor(const char *fn = nullptr) : IObjectConstructor(fn) {}
    [[nodiscard]] RuntimeObject *construct_impl() const override {
        return new T{};
    }
    [[nodiscard]] string_view class_name() const override {
        return type_string<T>();
    }
};
}// namespace vision::inline hotfix
