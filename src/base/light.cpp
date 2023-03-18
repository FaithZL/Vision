//
// Created by Zero on 18/03/2023.
//

#include "light.h"
#include "mgr/scene.h"

namespace vision {

Light::Light(const LightDesc &desc, LightType light_type)
    : Node(desc), _type(light_type), _color(_scene->create_slot(desc.color_slot)) {}

LightSample IPointLight::sample_Li(const LightSampleContext &p_ref, Float2 u,
                                   const SampledWavelengths &swl) const noexcept {
    LightSample ret{swl.dimension()};
    LightEvalContext p_light;
    p_light.pos = position();
    ret.eval = evaluate(p_ref, p_light, swl);
    Float3 wi_un = position() - p_ref.pos;
    ret.p_light = p_light.pos;
    return ret;
}
}// namespace vision