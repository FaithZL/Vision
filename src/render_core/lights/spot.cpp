//
// Created by Zero on 26/11/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"

namespace vision {

//    "type" : "spot",
//    "param" : {
//        "angle" : 20,
//        "falloff" : 2,
//        "direction" : [0.2, -1, 0],
//        "color" : [1, 1, 1],
//        "position" : [-0.5, 1.9, 0],
//        "scale" : 1
//    }
class SpotLight : public IPointLight {
private:
    Serial<float3> _position;
    Serial<float3> _direction;
    Serial<float> _angle;
    // falloff angle range
    Serial<float> _falloff;

public:
    explicit SpotLight(const LightDesc &desc)
        : IPointLight(desc),
          _position(desc["position"].as_float3()),
          _angle(radians(ocarina::clamp(desc["angle"].as_float(45.f), 1.f, 89.f))),
          _falloff(radians(ocarina::clamp(desc["falloff"].as_float(10.f), 0.f, _angle.hv()))),
          _direction(normalize(desc["direction"].as_float3(float3(0, 0, 1)))) {}
    OC_SERIALIZABLE_FUNC(IPointLight, _position, _direction, _angle, _falloff)
    VS_MAKE_PLUGIN_NAME_FUNC
    bool render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        _changed |= widgets->input_float3("direction", &_direction.hv());
        _changed |= widgets->slider_float("angle", &_angle.hv(), radians(1.f), radians(89.f));
        _changed |= widgets->slider_float("fall off", &_falloff.hv(), 0.001, _angle.hv());
        return _changed;
    }
    [[nodiscard]] float3 power() const noexcept override {
        return 2 * Pi * average() * (1 - .5f * (_angle.hv() * 2 + _falloff.hv()));
    }
    [[nodiscard]] Float3 position() const noexcept override { return *_position; }
    [[nodiscard]] float3 &host_position() noexcept override {
        return _position.hv();
    }
    [[nodiscard]] Float falloff(Float cos_theta) const noexcept {
        Float falloff_start = max(0.f, *_angle - *_falloff);
        Float cos_angle = cos(*_angle);
        Float cos_falloff_start = cos(falloff_start);
        cos_theta = clamp(cos_theta, cos_angle, cos_falloff_start);
        Float factor = (cos_theta - cos_angle) / (cos_falloff_start - cos_angle);
        Float ret = Pow<4>(factor);
        return ret;
    }
    [[nodiscard]] Float3 direction(const LightSampleContext &p_ref) const noexcept override {
        return *_direction;
    }
    [[nodiscard]] SampledSpectrum Le(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        Float3 w_un = p_ref.pos - position();
        Float3 w = normalize(w_un);
        SampledSpectrum value = _color.eval_illumination_spectrum(p_light.uv, swl).sample * scale();
        return value / length_squared(w_un) * falloff(dot(*_direction, w));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SpotLight)