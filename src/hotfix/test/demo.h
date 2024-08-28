//
// Created by Zero on 2024/8/21.
//

#pragma once

#include "core/stl.h"
#include "hotfix/object.h"
#include "hotfix/hotfix.h"
#include "test.h"

namespace vision::inline hotfix {

struct Demo : RuntimeObject {
private:
    SP<Test> test;
    int attr_int{1};
    float attr_float{2.5};

public:
    Demo();
    void clear() noexcept {
        test->clear();
        attr_int = 0;
        attr_float = 0;
    }
    void serialize(SP<Serializable> output) const noexcept override;
    void deserialize(SP<Serializable> input) noexcept override;
};

}// namespace vision::inline hotfix