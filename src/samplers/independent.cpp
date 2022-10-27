//
// Created by Zero on 09/09/2022.
//

#include "base/sampler.h"
#include "rhi/common.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;

class IndependentSampler : public Sampler {
private:
    Buffer<uint> _states;
    Uint _cur_state{nullptr};
public:
    explicit IndependentSampler(const SamplerDesc &desc) : Sampler(desc) {}

    void start_pixel_sample(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept override {

    }
    void prepare(RenderPipeline *rp) noexcept override {

    }

    [[nodiscard]] Float next_1d() const noexcept override {
        return 0.f;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IndependentSampler)