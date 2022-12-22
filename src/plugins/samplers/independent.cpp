//
// Created by Zero on 09/09/2022.
//

#include "base/sampler.h"
#include "rhi/common.h"
#include "base/mgr/render_pipeline.h"
#include "math/util.h"

namespace vision {
using namespace ocarina;

class IndependentSampler : public Sampler {
private:
    optional<Uint> _state{};

public:
    explicit IndependentSampler(const SamplerDesc &desc) : Sampler(desc) {}

    void start_pixel_sample(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept override {
        _state = tea<D>(tea<D>(pixel.x, pixel.y), tea<D>(sample_index, dim));
    }

    [[nodiscard]] Float next_1d() noexcept override {
        Float ret = lcg<D>(*_state);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IndependentSampler)