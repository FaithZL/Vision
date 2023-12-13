//
// Created by Zero on 18/03/2023.
//

#include "light.h"
#include "base/mgr/scene.h"

namespace vision {

Light::Light(const LightDesc &desc, LightType light_type)
    : Node(desc), _type(light_type),
      _color(scene().create_slot(desc.color)),
      _scale(desc["scale"].as_float(1.f)) {}

void IAreaLight::set_instance(const vision::ShapeInstance *inst) noexcept {
    _instance = inst;
}

ShapeInstance *IAreaLight::instance() const noexcept {
    return scene().get_instance(_inst_idx.hv());
}

LightSample IPointLight::sample_Li(const LightSampleContext &p_ref, Float2 u,
                                   const SampledWavelengths &swl) const noexcept {
    LightSample ret{swl.dimension()};
    LightEvalContext p_light;
    p_light.ng = direction(p_ref);
    p_light.pos = position();
    ret.eval = evaluate(p_ref, p_light, swl);
    ret.p_light = p_light.pos;
    return ret;
}
}// namespace vision