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


string Demo::get_string() const {
    return "Demo::string";
}

void Demo::serialize(SP<ISerialized> output) const noexcept {
    std::cout << "Demo::serialize" << endl;
    output->serialize("attr_int", attr_int);
    output->serialize("attr_float", attr_float);
    output->serialize("test", test.get());
}

void Demo::deserialize(SP<ISerialized> input) noexcept {
    std::cout << "Demo::deserialize" << endl;
    input->deserialize("attr_int", addressof(attr_int));
    input->deserialize("attr_float", addressof(attr_float));
    input->deserialize("test", test.get());
}

void Demo::on_update(const vector<const IObjectConstructor*> &constructors) noexcept {

}

}// namespace vision::inline hotfix

VS_REGISTER_HOTFIX(vision::hotfix, Demo)