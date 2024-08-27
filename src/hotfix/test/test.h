//
// Created by Zero on 2024/8/17.
//

#pragma once

#include "core/stl.h"
#include "hotfix/object.h"
#include "hotfix/hotfix.h"

namespace vision::inline hotfix {

struct Test : RuntimeObject {
public:
    int attr_int{};
    float attr_float{};

public:
    void serialize(vision::Serializable *serializer) const noexcept override;
    void deserialize(vision::Serializable *serializer) const noexcept override;
};

}// namespace vision::inline hotfix
