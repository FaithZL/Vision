//
// Created by Zero on 2024/8/17.
//

#include "test.h"
#include "hotfix/hotfix.h"

namespace vision ::inline hotfix {

void Test::serialize(vision::Serializer *serializer) const noexcept {
    std::cout << "Test::serialize" << endl;
    serializer->serialize(this, "attr_int", attr_int);
    serializer->serialize(this, "attr_float", attr_float);
}

void Test::deserialize(vision::Serializer *serializer) const noexcept {
}

}// namespace vision::inline hotfix

VS_REGISTER_CURRENT_PATH(1, "vision-hotfix-test.dll")

VS_REGISTER_HOTFIX(vision, Test)