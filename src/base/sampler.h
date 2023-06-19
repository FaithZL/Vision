//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "base/sensor/sensor.h"
#include "base/sensor/filter.h"

namespace vision {
using namespace ocarina;
class Sampler : public Node {
public:
    using Desc = SamplerDesc;

protected:
    uint _spp{1u};

public:
    explicit Sampler(const SamplerDesc &desc)
        : Node(desc), _spp(desc["spp"].as_uint(1u)) {}
    [[nodiscard]] virtual Float next_1d() noexcept = 0;
    virtual void start_pixel_sample(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept = 0;
    [[nodiscard]] virtual Float2 next_2d() noexcept {
        Float x = next_1d();
        Float y = next_1d();
        return make_float2(x, y);
    }
    [[nodiscard]] SensorSample sensor_sample(const Uint2 &pixel, const Filter *filter) {
        SensorSample ss;
        FilterSample fs = filter->sample(next_2d());
        ss.p_film = make_float2(pixel) + make_float2(0.5f) + fs.p;
        ss.u2 = next_2d();
        ss.time = next_1d();
        ss.filter_weight = fs.weight;
        return ss;
    }
};
}// namespace vision