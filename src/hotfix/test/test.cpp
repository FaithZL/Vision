//
// Created by Zero on 2024/8/17.
//

#include "test.h"
#include "hotfix/hotfix.h"

namespace vision ::inline hotfix {


void Test::serialize(SP<ISerialized> output) const noexcept {
#ifdef HOTFIX_FLAG
    std::cout << "Test::serialize  new-----" << endl;
#else
    std::cout << "Test::serialize" << endl;
#endif
    output->serialize("attr_int", attr_int);
    output->serialize("attr_float", attr_float);
#ifdef HOTFIX_FLAG
    output->serialize("attr_double", attr_double);
#endif
}

void Test::fill() noexcept {
    attr_float = 6.66;
    attr_int = 12;
#ifdef HOTFIX_FLAG
    attr_double = 678;
#endif
}

void Test::clear() noexcept {
    attr_int = 0;
    attr_float = 0;
#ifdef HOTFIX_FLAG
    attr_double = 0;
#endif
}

void Test::print() const noexcept {
    std::cout << "test print begin" << std::endl;
    std::cout << "      attr_float = " << attr_float << endl;
    std::cout << "      attr_int = " << attr_int << endl;
#ifdef HOTFIX_FLAG
    std::cout << "      attr_double = " << attr_double << endl;
#endif
    std::cout << "test print end" << std::endl;
}

void Test::deserialize(SP<ISerialized> input) noexcept {
#ifdef HOTFIX_FLAG
    std::cout << "Test::deserialize  new-----" << endl;
#else
    std::cout << "Test::deserialize" << endl;
#endif
    input->deserialize("attr_int", addressof(attr_int));
    input->deserialize("attr_float", addressof(attr_float));
#ifdef HOTFIX_FLAG
    input->deserialize("attr_double", addressof(attr_double));
#endif
}

string Test::get_string() const {
    return "Test::get_string";
}

}// namespace vision::inline hotfix

VS_REGISTER_CURRENT_PATH(1, "vision-hotfix-test.dll")

VS_REGISTER_HOTFIX(vision, Test)