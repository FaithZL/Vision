//
// Created by Zero on 2024/8/21.
//

#include "demo.h"
#include "core/vs_header.h"
#include "hotfix/hotfix.h"
#include "hotfix/module_interface.h"

namespace vision::inline hotfix {

Demo::Demo()
    :test(make_shared<Test>()) {}

void Demo::serialize(vision::Serializable *serializer) const noexcept {
    std::cout << "Demo::serialize" << endl;
//    serializer->serialize(this, "attr_int", attr_int);
//    serializer->serialize(this, "attr_float", attr_float);
}

void Demo::deserialize(vision::Serializable *serializer) const noexcept {
}

}// namespace vision::inline hotfix

VS_REGISTER_HOTFIX(vision, Demo)