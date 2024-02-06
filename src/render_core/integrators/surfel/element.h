//
// Created by Zero on 2024/2/6.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/dsl.h"

namespace vision::surfel {
using namespace ocarina;
struct Element {
    array<float, 3> pos{};
    array<float, 3> ng{};
    float radius{};
};
}// namespace vision::surfel

// clang-format off
OC_STRUCT(vision::surfel::Element, pos, ng, radius) {
    [[nodiscard]] auto normal() const noexcept {
        return ng.as_vec();
    }
    [[nodiscard]] auto position() const noexcept {
        return pos.as_vec();
    }
};
// clang-format on

namespace vision::surfel {

}