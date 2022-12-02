//
// Created by Zero on 22/10/2022.
//

#include "lightsampler.h"
#include "render_pipeline.h"

namespace vision {

LightSampler::LightSampler(const LightSamplerDesc &desc)
    : Node(desc), _env_prob(clamp(desc.env_prob, 0.01f, 0.99f)) {
    for (const LightDesc &light_desc : desc.light_descs) {
        Light *light = desc.scene->load<Light>(light_desc);
        add_light(light);
        if (light->type() == LightType::Infinite) {
            _env_light = light;
        }
    }
    std::sort(_lights.begin(), _lights.end(), [&](Light *a, Light *b) {
        return a->type() > b->type();
    });
}

LightEval LightSampler::evaluate_hit(const LightSampleContext &p_ref,
                                      const Interaction &si) const noexcept {
    LightEval ret;
    dispatch_light(si.light_id, [&](const Light *light) {
        if (light->type() != LightType::Area) { return; }
        LightEvalContext p_light{si};
        p_light.PDF_pos *= light->PMF(si.prim_id);
        ret = light->evaluate(p_ref, p_light);
    });
    Float pmf = PMF(p_ref, si.light_id);
    ret.pdf *= pmf;
    return ret;
}

LightEval LightSampler::evaluate_miss(const LightSampleContext &p_ref, Float3 wi) const noexcept {
    LightEval ret;
    LightEvalContext p_light{p_ref.pos + wi};
    ret = env_light()->evaluate(p_ref, p_light);
    Float pmf = 1.f / light_num();
    ret.pdf *= pmf;
    return ret;
}

void LightSampler::dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept {
    _lights.dispatch(id, func);
}

}// namespace vision