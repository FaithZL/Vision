//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "sample.h"
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
    [[nodiscard]] virtual Float next_1d() const noexcept = 0;
    virtual void start_pixel_sample(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept = 0;
    [[nodiscard]] virtual Float2 next_2d() const noexcept { return make_float2(next_1d(), next_1d()); }
    [[nodiscard]] SensorSample sensor_sample(const Uint2 &pixel, const Filter *filter = nullptr) {
        SensorSample ss;
        ss.p_film = make_float2(pixel) + make_float2(0.5f);
        return ss;
    }
};
}// namespace vision