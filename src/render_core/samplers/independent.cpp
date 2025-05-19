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
    optional<Uint> state_{};

public:
    IndependentSampler() = default;
    explicit IndependentSampler(const SamplerDesc &desc) : Sampler(desc) {}
    VS_HOTFIX_MAKE_RESTORE(Sampler, state_)
    void load_data() noexcept override {
        state_.emplace(Uint{0u});
    }
    void start(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept override {
        Uint state = tea<D>(tea<D>(pixel.x, pixel.y), tea<D>(sample_index, dim));
        try_load_data();
        state_ = state;
    }
    void temporary(const ocarina::function<void(Sampler *)> &func) noexcept override {
        try_load_data();
        Uint temp_state = *state_;
        func(this);
        *state_ = temp_state;
    }
    [[nodiscard]] bool is_valid() const noexcept override {
        return state_ && state_->is_valid();
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] Float next_1d() noexcept override {
        Float ret = lcg<D>(*state_);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, IndependentSampler)