//
// Created by Zero on 28/10/2022.
//

#include "base/lightsampler.h"
#include "base/mgr/render_pipeline.h"
#include "base/sampler.h"

namespace vision {
class LightBVHSampler : public LightSampler {
public:
    explicit LightBVHSampler(const LightSamplerDesc &desc)
        : LightSampler(desc) {}
};
}// namespace vision