//
// Created by Zero on 2024/8/17.
//

#include "test.h"
#include "hotfix/hotfix.h"

namespace vision ::inline hotfix {

void Test::serialize(SP<Serializable> output) const noexcept {
    std::cout << "Test::serialize" << endl;
    //    output->serialize("attr_int", attr_int);
    //    output->serialize(this, "attr_float", attr_float);
}

void Test::deserialize(SP<Serializable> input) const noexcept {
}

}// namespace vision::inline hotfix

VS_REGISTER_CURRENT_PATH(1, "vision-hotfix-test.dll")

VS_REGISTER_HOTFIX(vision, Test)