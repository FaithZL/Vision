//
// Created by Zero on 2024/8/17.
//

#include "test.h"
#include "hotfix/hotfix.h"

namespace vision ::inline hotfix {


void Test::serialize(SP<ISerialized> output) const noexcept {
    std::cout << "Test::serialize" << endl;
    output->serialize("attr_int", attr_int);
    output->serialize("attr_float", attr_float);
}

void Test::deserialize(SP<ISerialized> input) noexcept {
    std::cout << "Test::deserialize" << endl;
    input->deserialize("attr_int", addressof(attr_int));
    input->deserialize("attr_float", addressof(attr_float));
}

string Test::get_string() const {
    return "Test::get_string";
}

}// namespace vision::inline hotfix

VS_REGISTER_CURRENT_PATH(1, "vision-hotfix-test.dll")

VS_REGISTER_HOTFIX(vision, Test)