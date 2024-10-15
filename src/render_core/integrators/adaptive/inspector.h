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
    float avg{};
    float var{};
    uint N{};

    void add(float x) noexcept {
        using ocarina::sqr;
        float new_avg = (avg * N + x) / (N + 1);
        float new_var = (N * var + N * sqr(avg - new_avg) + sqr(x - avg) + 2 * (x - avg) * (avg - new_avg) + sqr(avg - new_avg)) / (N + 1);
        N += 1;
        avg = new_avg;
        var = new_var;
    }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision, VarianceStats, avg, var, N) {};
// clang-format on

namespace vision {

class ConvergenceInspector final : public GUI, public RuntimeObject, Encodable<> {
private:
    EncodedData<float> threshold_;
    EncodedData<uint> start_index_;

public:
    ConvergenceInspector() = default;
    ConvergenceInspector(float threshold, uint start_index)
        : threshold_(threshold), start_index_(start_index){};
    ~ConvergenceInspector() override = default;
};

}// namespace vision