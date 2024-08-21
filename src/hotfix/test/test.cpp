//
// Created by Zero on 2024/8/17.
//

#include "test.h"
#include "hotfix/hotfix.h"

namespace vision ::inline hotfix {

void Test::serialize(vision::Serializer *serializer) const noexcept {
}

void Test::deserialize(vision::Serializer *serializer) const noexcept {
}

IObjectConstructor *Test::constructor() const noexcept {
    return nullptr;
}

}// namespace vision::inline hotfix

VS_REGISTER_CURRENT_PATH(1, "vision-hotfix-test.dll")