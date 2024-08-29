//
// Created by Zero on 2024/8/17.
//

#pragma once

#include "core/stl.h"
#include "hotfix/object.h"
#include "hotfix/hotfix.h"

namespace vision::inline hotfix {

struct Test : RuntimeObject {
public:
    int attr_int{5};
    float attr_float{6.6};

public:
    void clear() noexcept {
        attr_int = 0;
        attr_float = 0;
    }
    [[nodiscard]] string get_string() const;
    void fill() noexcept {
        attr_float = 6.66;
        attr_int = 12;
    }

    void print() const noexcept {
        std::cout << "test print begin" << std::endl;
        std::cout << "      attr_float = " << attr_float << endl;
        std::cout << "      attr_int = " << attr_int << endl;
        std::cout << "test print end" << std::endl;
    }

    void serialize(SP<Serializable> output) const noexcept override;
    void deserialize(SP<Serializable> input) noexcept override;
};

}// namespace vision::inline hotfix
