//
// Created by Zero on 09/09/2022.
//

#include "base/lightsampler.h"
#include "core/render_pipeline.h"

namespace vision {
class UniformLightSampler : public LightSampler {
public:
    explicit UniformLightSampler(const LightSamplerDesc &desc) : LightSampler(desc) {}
    [[nodiscard]] Float PMF(const LightSampleContext &lsc, const Uint &id) const noexcept override {
        return 1.f / light_num();
    }
    [[nodiscard]] SampledLight select_light(const LightSampleContext &lsc, const Float &u) const noexcept override {
        SampledLight ret;
        ret.light_id = min(u * float(light_num()), float(light_num()) - 1);
        ret.PMF = 1.f / light_num();
        return ret;
    }
    [[nodiscard]] LightSample sample(const LightSampleContext &lsc, const Float &u_light,
                                     const Float2 &u_surface) const noexcept override {
        LightSample ret;
        SampledLight sampled_light = select_light(lsc, u_light);
        RenderPipeline *rp = _scene->render_pipeline();
        rp->dispatch<Light>(sampled_light.light_id, _lights, [&](const Light *light) {
            ret = light->sample_Li(lsc, u_surface);
            ret.eval.pdf *= sampled_light.PMF;
        });
        return ret;
    }
};
}

VS_MAKE_CLASS_CREATOR(vision::UniformLightSampler)