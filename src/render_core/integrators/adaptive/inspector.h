//
// Created by Zero on 2024/10/15.
//

#pragma once

#include "core/stl.h"
#include "hotfix/hotfix.h"
#include "dsl/dsl.h"
#include "UI/GUI.h"
#include "core/parameter_set.h"
#include "base/mgr/global.h"

namespace vision {

struct alignas(16u) VarianceStats {
    float avg{};
    float var{};
    uint N{};

    template<EPort p>
    static void add_sample_impl(oc_uint<p> &N, oc_float<p> &avg, oc_float<p> &var, const oc_float<p> &x) noexcept {
        using ocarina::sqr;
        oc_float<p> new_avg = (avg * N + x) / (N + 1);
        oc_float<p> new_var = (N * var + N * sqr(avg - new_avg) + sqr(x - avg) + 2 * (x - avg) * (avg - new_avg) + sqr(avg - new_avg)) / (N + 1);
        N += 1;
        avg = new_avg;
        var = new_var;
    }

    void add(float x) noexcept {
        add_sample_impl<H>(N, avg, var, x);
    }
    [[nodiscard]] float relative_variance() noexcept { return var / avg; }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision, VarianceStats, avg, var, N) {
    void add(const Float &x) noexcept {
        vision::VarianceStats::add_sample_impl<D>(N, avg, var, x);
    }
    [[nodiscard]] Float relative_variance() noexcept { return var / avg; }
};
// clang-format on

namespace vision {

class ConvergenceInspector : public GUI, public RuntimeObject, Encodable<>, Context {
private:
    EncodedData<float> threshold_;
    EncodedData<uint> start_index_;
    RegistrableBuffer<VarianceStats> variance_stats_{};

public:
    ConvergenceInspector() = default;
    explicit ConvergenceInspector(const ParameterSet &ps);
    ConvergenceInspector(float threshold, uint start_index)
        : threshold_(threshold), start_index_(start_index){};
    OC_ENCODABLE_FUNC(Encodable<>, threshold_, start_index_)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void prepare() noexcept;
    void add_sample(const Uint2 &pixel, const Float3 &value, const Uint &frame_index) noexcept;
    [[nodiscard]] Bool is_convergence(const Uint &frame_index) const noexcept;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    ~ConvergenceInspector() override = default;
};

}// namespace vision