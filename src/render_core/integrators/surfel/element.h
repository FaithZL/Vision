//
// Created by Zero on 2024/2/6.
//

#pragma once

#include "math/basic_types.h"
#include "dsl/dsl.h"

namespace vision::surfel {
using namespace ocarina;
struct Element {
    array<float, 3> pos{};
    array<float, 3> ng{};
    float radius{};
    uint transform_id{};
    array<float, 3> irr{};
    float depth{};
};
}// namespace vision::surfel

// clang-format off
OC_STRUCT(vision::surfel, Element, pos, ng, radius, transform_id, irr, depth) {
    [[nodiscard]] auto normal() const noexcept {
        return ng.as_vec();
    }
    [[nodiscard]] auto position() const noexcept {
        return pos.as_vec();
    }
    [[nodiscard]] auto irradiance() const noexcept {
        return irr.as_vec();
    }
};
// clang-format on

namespace vision::surfel {

}