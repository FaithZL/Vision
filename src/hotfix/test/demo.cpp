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

void Demo::update(const vision::IObjectConstructor *constructor) noexcept {
    if (constructor->class_name() != test->class_name()) {
        return;
    }
    auto new_obj = constructor->construct_shared<Test>();
    auto serialized_data = test->serialized_data();
    new_obj->deserialize(serialized_data);
    test = new_obj;
}

void Demo::on_update(const vector<const IObjectConstructor*> &constructors) noexcept {
    for (const auto &item : constructors) {
        update(item);
    }
}

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

}// namespace vision::inline hotfix

VS_REGISTER_HOTFIX(vision::hotfix, Demo)