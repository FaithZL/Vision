//
// Created by Zero on 09/09/2022.
//

#include "base/sampler.h"

namespace vision {
using namespace ocarina;

class IndependentSampler : public Sampler {
public:
    explicit IndependentSampler(const SamplerDesc &desc) : Sampler(desc) {}

    [[nodiscard]] Float next_1d() const noexcept override {
        return 0.f;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IndependentSampler)