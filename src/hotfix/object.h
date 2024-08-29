//
// Created by Zero on 2024/7/31.
//

#pragma once

#include "core/hash.h"
#include "macro.h"

namespace vision::inline hotfix {
using namespace ocarina;
class Serializer;

class Serializable;

class RuntimeObject : public Hashable {
public:
    virtual void serialize(SP<Serializable> output) const noexcept = 0;
    virtual void deserialize(SP<Serializable> input) noexcept = 0;
    virtual ~RuntimeObject() = default;
};

class IObjectConstructor {
protected:
    string_view filename_{};

public:
    explicit IObjectConstructor(const char *fn) : filename_(fn) {}
    OC_MAKE_MEMBER_GETTER(filename, )
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
