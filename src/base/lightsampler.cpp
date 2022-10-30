//
// Created by Zero on 22/10/2022.
//

#include "lightsampler.h"
#include "core/render_pipeline.h"

namespace vision {

LightSampler::LightSampler(const LightSamplerDesc &desc) : Node(desc) {
    for (const LightDesc &light_desc : desc.light_descs) {
        add_light(desc.scene->load<Light>(light_desc));
    }
}

Evaluation LightSampler::evaluate_hit(const LightSampleContext &p_ref,
                                      const SurfaceInteraction &si) const noexcept {
    Evaluation ret;
    dispatch_light(si.light_id, [&](const Light *light) {
        ret = light->evaluate(p_ref, si);
    });
    Float pmf = PMF(p_ref, si.light_id);
    ret.pdf *= pmf;
    return ret;
}

void LightSampler::dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept {
    RenderPipeline *rp = _scene->render_pipeline();
    rp->dispatch<Light>(id, _lights, func);
}

}// namespace vision