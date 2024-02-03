//
// Created by Zero on 09/09/2022.
//

#include "base/sampler.h"
#include "rhi/common.h"
#include "base/mgr/pipeline.h"
#include "math/util.h"

namespace vision {
using namespace ocarina;

class IndependentSampler : public Sampler {
private:
    optional<Uint> _state{};

public:
    explicit IndependentSampler(const SamplerDesc &desc) : Sampler(desc) {}
    void load_data() noexcept override {
        _state.emplace(Uint{0u});
    }
    void start(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept override {
        Uint state = tea<D>(tea<D>(pixel.x, pixel.y), tea<D>(sample_index, dim));
        if (is_valid()) {
            _state = state;
        } else {
            _state.emplace(state);
        }
    }
    [[nodiscard]] bool is_valid() const noexcept override {
        return _state && _state->is_valid();
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] Float next_1d() noexcept override {
        Float ret = lcg<D>(*_state);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IndependentSampler)