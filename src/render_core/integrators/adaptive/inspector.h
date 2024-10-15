//
// Created by Zero on 2024/10/15.
//

#pragma once

#include "core/stl.h"
#include "hotfix/hotfix.h"
#include "dsl/dsl.h"
#include "UI/GUI.h"

namespace vision {
struct alignas(16u) VarianceStats {
    float average{};
    float variance{};
    uint count{};

    void add(float value) noexcept {
        average = (average * count + value) / (count + 1);
        count += 1;
    }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision, VarianceStats, average, variance, count) {};
// clang-format on

namespace vision {

class ConvergenceInspector : public GUI, public RuntimeObject {
private:

};

}// namespace vision