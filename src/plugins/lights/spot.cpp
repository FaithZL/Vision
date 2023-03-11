//
// Created by Zero on 26/11/2022.
//

#include "base/light.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
class SpotLight : public IPointLight {
private:
    float3 _intensity;
    float3 _position;
    float3 _direction;
    float _angle;
    // falloff angle range
    float _falloff;

public:
    explicit SpotLight(const LightDesc &desc)
        : IPointLight(desc),
          _intensity(desc["intensity"].as_float3(make_float3(1.f)) * desc["scale"].as_float(1.f)),
          _position(desc["position"].as_float3()),
          _angle(radians(ocarina::clamp(desc["angle"].as_float(45.f), 1.f, 89.f))),
          _falloff(radians(ocarina::clamp(desc["falloff"].as_float(10.f), 0.f, _angle))),
          _direction(normalize(desc["direction"].as_float3(float3(0, 0, 1)))) {}
    [[nodiscard]] float3 position() const noexcept override { return _position; }
    [[nodiscard]] Float falloff(Float cos_theta) const noexcept {
        float falloff_start = max(0.f, _angle - _falloff);
        float cos_angle = cos(_angle);
        float cos_falloff_start = cos(falloff_start);
        cos_theta = clamp(cos_theta, cos_angle, cos_falloff_start);
        Float factor = (cos_theta - cos_angle) / (cos_falloff_start - cos_angle);
        Float ret = Pow<4>(factor);
        return ret;
    }
    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                             const LightEvalContext &p_light,
                             const SampledWavelengths &swl) const noexcept override {
        Float3 w_un = p_ref.pos - _position;
        Float3 w = normalize(w_un);
        SampledSpectrum value = spectrum().decode_to_illumination(_intensity, swl).sample;
        return value / length_squared(w_un) * falloff(dot(_direction, w));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SpotLight)