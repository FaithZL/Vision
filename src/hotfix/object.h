//
// Created by Zero on 2024/7/31.
//

#pragma once

#include "serializer.h"
#include "core/hash.h"

namespace vision::inline hotfix {
using namespace ocarina;

class RuntimeObject;

class IObjectConstructor {
public:
    virtual RuntimeObject *construct() = 0;
    virtual string_view class_name() = 0;
    virtual string_view file_name() = 0;
};

class RuntimeObject : public Hashable {
public:
    virtual void serialize(Serializer *serializer) const noexcept = 0;
    virtual void deserialize(Serializer *serializer) const noexcept = 0;
    virtual IObjectConstructor *constructor() const noexcept = 0;
};
}// namespace vision::inline hotfix
