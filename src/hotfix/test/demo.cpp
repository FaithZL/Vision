//
// Created by Zero on 2024/8/21.
//

#include "demo.h"
#include "core/vs_header.h"
#include "hotfix/hotfix.h"
#include "hotfix/module_interface.h"

namespace vision::inline hotfix {

Demo::Demo()
    : test() {}

Demo::~Demo() {
    std::cout << "del Demo" << endl;
}

void Demo::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    if (constructor->class_name() != test->class_name()) {
        return;
    }
    auto new_obj = constructor->construct_shared<Test>();
    HotfixSystem::instance().serializer().erase_old_object(test.get());
    new_obj->restore(test.get());
    test = new_obj;
}

void Demo::restore(const vision::RuntimeObject *old_obj) noexcept {
//    RuntimeObject::restore(old_obj);
    VS_HOTFIX_MOVE_ATTRS(Demo, attr_int, attr_float, test)
}

void Demo::clear() noexcept {
    test->clear();
    attr_int = 0;
    attr_float = 0;
}

void Demo::print() noexcept {
    std::cout << "demo print begin" << std::endl;
    std::cout << "      attr_float = " << attr_float << endl;
    std::cout << "      attr_int = " << attr_int << endl;
    test->print();
    std::cout << "demo print end" << std::endl;
}

void Demo::fill() noexcept {
    test->fill();
    attr_float = 57.9;
    attr_int = 789;
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