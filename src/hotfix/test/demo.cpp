//
// Created by Zero on 2024/8/21.
//

#include "demo.h"
#include "core/vs_header.h"
#include "hotfix/hotfix.h"
#include "hotfix/module_interface.h"

namespace vision::inline hotfix {

Demo::Demo()
    : test(make_shared<Test>()) {}

void Demo::serialize(SP<Serializable> output) const noexcept {
    std::cout << "Demo::serialize" << endl;
    output->serialize("attr_int", attr_int);
    output->serialize("attr_float", attr_float);
    output->serialize("test", test);
}

void Demo::deserialize(SP<Serializable> input) const noexcept {
}

}// namespace vision::inline hotfix

VS_REGISTER_HOTFIX(vision::hotfix, Demo)