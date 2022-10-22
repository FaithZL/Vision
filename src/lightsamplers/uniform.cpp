//
// Created by Zero on 09/09/2022.
//

#include "base/lightsampler.h"

namespace vision {
class UniformLightSampler : public LightSampler {
public:
    explicit UniformLightSampler(const LightSamplerDesc &desc) : LightSampler(desc) {}
    [[nodiscard]] Float PMF(const Uint &id) const noexcept override { return 1.f / light_num(); }
    [[nodiscard]] SampledLight sample(const Float &u) const noexcept override {
        SampledLight ret;
        ret.light_id = min(u * float(light_num()), float(light_num()) - 1);
        ret.PMF = 1.f / light_num();
        return ret;
    }
};
}

VS_MAKE_CLASS_CREATOR(vision::UniformLightSampler)