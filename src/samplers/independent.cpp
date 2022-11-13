//
// Created by Zero on 09/09/2022.
//

#include "base/sampler.h"
#include "rhi/common.h"
#include "base/render_pipeline.h"
#include "math/util.h"

namespace vision {
using namespace ocarina;

class IndependentSampler : public Sampler {
private:
    optional<Uint> _state{};
    Callable<canonical_signature_t<decltype(lcg<H>)>> _lcg{lcg<D>};
    Callable<canonical_signature_t<decltype(tea<H>)>> _tea{tea<D>};

public:
    explicit IndependentSampler(const SamplerDesc &desc) : Sampler(desc) {}

    void start_pixel_sample(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept override {
        _state = _tea(_tea(pixel.x, pixel.y), _tea(sample_index, dim));
    }

    [[nodiscard]] Float next_1d() noexcept override {
        Float ret = _lcg(*_state);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IndependentSampler)