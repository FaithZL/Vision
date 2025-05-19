//
// Created by Zero on 09/09/2022.
//

#include "base/illumination/lightsampler.h"
#include "base/mgr/pipeline.h"
#include "base/sampler.h"

namespace vision {
class UniformLightSampler : public LightSampler {
protected:
    [[nodiscard]] Float PMF_(const LightSampleContext &lsc, const Uint &index) const noexcept override {
        if (env_separate_) {
            return 1.f / punctual_light_num();
        }
        return 1.f / light_num();
    }

public:
    UniformLightSampler() = default;
    explicit UniformLightSampler(const LightSamplerDesc &desc) : LightSampler(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] SampledLight select_light_(const LightSampleContext &lsc, const Float &u) const noexcept override {
        SampledLight ret;
        if (env_separate_) {
            ret.light_index = min(u * float(punctual_light_num()), float(punctual_light_num()) - 1);
            ret.light_index = correct_index(ret.light_index);
            ret.PMF = 1.f / punctual_light_num();
        } else {
            ret.light_index = min(u * float(light_num()), float(light_num()) - 1);
            ret.PMF = 1.f / light_num();
        }
        return ret;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, UniformLightSampler)