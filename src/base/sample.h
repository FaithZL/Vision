//
// Created by Zero on 29/10/2022.
//

#pragma once

#include "dsl/common.h"

namespace vision {
using namespace ocarina;
struct Evaluation {
    Float3 val;
    Float pdf{-1.f};
    [[nodiscard]] Bool valid() const noexcept {
        return pdf >= 0.f;
    }
};

}// namespace vision