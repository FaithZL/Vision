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

bool Light::render_UI(ocarina::Widgets *widgets) noexcept {
    string label = format("{} light: {}", impl_type().data(), _name.c_str());
    bool open = widgets->use_folding_header(label, [&] {
        _changed |= widgets->input_float_limit("scale", &_scale.hv(), 0, 100, 0.1, 2);
        widgets->use_tree("color node", [&]{
            _color->render_UI(widgets);
        });
        render_sub_UI(widgets);
    });
    return open;
}

void IAreaLight::set_instance(const vision::ShapeInstance *inst) noexcept {
    _instance = inst;
}

ShapeInstance *IAreaLight::instance() const noexcept {
    return scene().get_instance(_inst_idx.hv());
}

LightSample IPointLight::sample_wi(const LightSampleContext &p_ref, Float2 u,
                                   const SampledWavelengths &swl) const noexcept {
    LightSample ret{swl.dimension()};
    LightEvalContext p_light;
    p_light.ng = direction(p_ref);
    p_light.pos = position();
    ret.eval = evaluate_wi(p_ref, p_light, swl, LightEvalMode::All);
    ret.p_light = p_light.pos;
    return ret;
}
}// namespace vision