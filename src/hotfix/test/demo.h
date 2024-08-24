//
// Created by Zero on 2024/8/21.
//

#pragma once

#include "core/stl.h"
#include "hotfix/object.h"
#include "hotfix/hotfix.h"

namespace vision::inline hotfix {

struct Demo : RuntimeObject {
    void serialize(vision::Serializer *serializer) const noexcept override;
    void deserialize(vision::Serializer *serializer) const noexcept override;
};

}// namespace vision::inline hotfix