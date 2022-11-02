//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "sensor.h"
#include "filter.h"

namespace vision {
using namespace ocarina;
class Sampler : public Node {
public:
    using Desc = SamplerDesc;

protected:
    uint _spp{1u};

public:
    explicit Sampler(const SamplerDesc &desc) : Node(desc), _spp(desc.spp) {}
    [[nodiscard]] virtual Float next_1d() noexcept = 0;
    virtual void start_pixel_sample(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept = 0;
    [[nodiscard]] virtual Float2 next_2d() noexcept {
        Float x = next_1d();
        Float y = next_1d();
        return make_float2(x, y);
    }
    [[nodiscard]] SensorSample sensor_sample(const Uint2 &pixel, const Filter *filter = nullptr) {
        SensorSample ss;
        ss.p_film = make_float2(pixel) + next_2d();
        return ss;
    }
};
}// namespace vision