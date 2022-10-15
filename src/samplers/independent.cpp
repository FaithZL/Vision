//
// Created by Zero on 09/09/2022.
//

#include "base/sampler.h"

namespace vision {
using namespace ocarina;

class IndependentSampler : public Sampler {
public:
    explicit IndependentSampler(const SamplerDesc *desc) : Sampler(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::IndependentSampler)