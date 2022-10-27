//
// Created by Zero on 09/09/2022.
//

#include "base/sampler.h"
#include "rhi/common.h"
#include "core/render_pipeline.h"
#include "math/util.h"

namespace vision {
using namespace ocarina;

class IndependentSampler : public Sampler {
private:
    optional<Uint> _state{};
    Callable<float(uint &)> _lcg{lcg<D>};
    Callable<uint(uint, uint)> _tea{tea<D>};

public:
    explicit IndependentSampler(const SamplerDesc &desc) : Sampler(desc) {}

    void start_pixel_sample(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept override {
        _state = _tea(_tea(pixel.x, pixel.y), _tea(sample_index, dim));
    }

    [[nodiscard]] Float next_1d() noexcept override {
        return _lcg(*_state);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IndependentSampler)