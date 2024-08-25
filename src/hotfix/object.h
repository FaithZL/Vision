//
// Created by Zero on 2024/7/31.
//

#pragma once

#include "serializer.h"
#include "core/hash.h"

namespace vision::inline hotfix {
using namespace ocarina;

class RuntimeObject : public Hashable {
public:
    virtual void serialize(Serializer *serializer) const noexcept = 0;
    virtual void deserialize(Serializer *serializer) const noexcept = 0;
    virtual ~RuntimeObject() = default;
};

class IObjectConstructor {
public:
    [[nodiscard]] virtual RuntimeObject *construct() const = 0;
    static void destroy(RuntimeObject *obj) {
        delete obj;
    }
    [[nodiscard]] virtual string_view class_name() const = 0;
};

template<typename T>
requires std::derived_from<T, RuntimeObject>
class ObjectConstructor : public IObjectConstructor {
    [[nodiscard]] RuntimeObject *construct() const override {
        return new T{};
    }
    [[nodiscard]] string_view class_name() const override {
        return typeid(T).name();
    }
};
}// namespace vision::inline hotfix
