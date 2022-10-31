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

LightEval LightSampler::evaluate_hit(const LightSampleContext &p_ref,
                                      const SurfaceInteraction &si) const noexcept {
    LightEval ret;
    dispatch_light(si.light_id, [&](const Light *light) {
        LightEvalContext p_light{si};
        p_light.PDF_pos *= light->PMF(si.prim_id);
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