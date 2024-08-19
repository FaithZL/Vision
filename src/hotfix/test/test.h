//
// Created by Zero on 2024/8/17.
//

#pragma once

#include "core/stl.h"
#include "hotfix/object.h"
#include "hotfix/hotfix.h"

namespace vision::inline hotfix {

struct Test : RuntimeObject {
    void serialize(vision::Serializer *serializer) const noexcept override;
    void deserialize(vision::Serializer *serializer) const noexcept override;
    [[nodiscard]] IObjectConstructor * constructor() const noexcept override;
};

}// namespace vision::inline hotfix

