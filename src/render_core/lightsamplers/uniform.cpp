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
    [[nodiscard]] Float PMF(const LightSampleContext &lsc, const Uint &index) const noexcept override {
        return 1.f / light_num();
    }
    [[nodiscard]] SampledLight select_light(const LightSampleContext &lsc, const Float &u) const noexcept override {
        SampledLight ret;
        auto sample = [&] {
            ret.light_index = min(u * float(light_num()), float(light_num()) - 1);
            ret.PMF = 1.f / light_num();
            return ret;
        };
        if (env_prob() == 1) {

        }
        return sample();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::UniformLightSampler)