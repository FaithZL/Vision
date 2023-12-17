//
// Created by Zero on 09/09/2022.
//

#include "base/illumination/lightsampler.h"
#include "base/mgr/pipeline.h"
#include "base/sampler.h"

namespace vision {
class UniformLightSampler : public LightSampler {
public:
    explicit UniformLightSampler(const LightSamplerDesc &desc) : LightSampler(desc) {}
    [[nodiscard]] Float _PMF(const LightSampleContext &lsc, const Uint &index) const noexcept override {
        return 1.f / punctual_light_num();
    }
    [[nodiscard]] SampledLight _select_light(const LightSampleContext &lsc, const Float &u) const noexcept override {
        SampledLight ret;
        ret.light_index = min(u * float(punctual_light_num()), float(punctual_light_num()) - 1);
        ret.light_index = correct_index(ret.light_index);
        ret.PMF = 1.f / punctual_light_num();
        return ret;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::UniformLightSampler)