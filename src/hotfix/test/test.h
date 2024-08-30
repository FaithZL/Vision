//
// Created by Zero on 2024/8/17.
//

#pragma once

#include "core/stl.h"
#include "hotfix/object.h"
#include "hotfix/hotfix.h"

//#define HOTFIX_FLAG

namespace vision::inline hotfix {

struct Test : RuntimeObject {
public:
    int attr_int{};
    float attr_float{};
#ifdef HOTFIX_FLAG
    double attr_double{};
#endif
public:
    __declspec(noinline) void clear() noexcept {
        attr_int = 0;
        attr_float = 0;
#ifdef HOTFIX_FLAG
        attr_double = 0;
#endif
    }
    [[nodiscard]] string get_string() const;
    __declspec(noinline) void fill() noexcept {
        attr_float = 6.66;
        attr_int = 12;
#ifdef HOTFIX_FLAG
        attr_double = 678;
#endif
    }

    __declspec(noinline) void print() const noexcept {
        std::cout << "test print begin" << std::endl;
        std::cout << "      attr_float = " << attr_float << endl;
        std::cout << "      attr_int = " << attr_int << endl;
#ifdef HOTFIX_FLAG
        std::cout << "      attr_double = " << attr_double << endl;
#endif
        std::cout << "test print end" << std::endl;
    }

    void serialize(SP<ISerialized> output) const noexcept override;
    void deserialize(SP<ISerialized> input) noexcept override;
};

}// namespace vision::inline hotfix
