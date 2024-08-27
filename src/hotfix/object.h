//
// Created by Zero on 2024/7/31.
//

#pragma once

#include "core/hash.h"

#define VS_REGISTER_HOTFIX(NS, Class)                                                                          \
    namespace {                                                                                                \
    struct ConstructorRegistrar {                                                                              \
        ConstructorRegistrar() {                                                                               \
            using namespace vision::hotfix;                                                                    \
            ModuleInterface::instance().add_constructor(ocarina::make_shared<ObjectConstructor<NS::Class>>()); \
        }                                                                                                      \
    };                                                                                                         \
    static ConstructorRegistrar s_##Class##_registrar;                                                         \
    }// namespace

namespace vision::inline hotfix {
using namespace ocarina;
class Serializer;

class Serializable;

class RuntimeObject : public Hashable {
public:
    virtual void serialize(SP<Serializable> output) const noexcept = 0;
    virtual void deserialize(SP<Serializable> input) const noexcept = 0;
    virtual ~RuntimeObject() = default;
};

class IObjectConstructor {
public:
    template<typename T = RuntimeObject>
    [[nodiscard]] T *construct() const {
        return dynamic_cast<T *>(construct_impl());
    }
    [[nodiscard]] virtual RuntimeObject *construct_impl() const = 0;
    static void destroy(RuntimeObject *obj) {
        delete obj;
    }
    [[nodiscard]] virtual string_view class_name() const = 0;
};

template<typename T>
requires std::derived_from<T, RuntimeObject>
class ObjectConstructor : public IObjectConstructor {
    [[nodiscard]] RuntimeObject *construct_impl() const override {
        return new T{};
    }
    [[nodiscard]] string_view class_name() const override {
        return typeid(T).name();
    }
};
}// namespace vision::inline hotfix
