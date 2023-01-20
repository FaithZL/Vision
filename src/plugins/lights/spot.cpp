//
// Created by Zero on 26/11/2022.
//

#include "base/light.h"

namespace vision {
class SpotLight : public IPointLight {
private:
    VSColor _intensity;
    float3 _position;
    float3 _direction;
    float _angle;
    // falloff angle range
    float _falloff;

public:
    explicit SpotLight(const LightDesc &desc)
        : IPointLight(desc),
          _intensity(desc.intensity),
          _position(desc.position),
          _angle(radians(desc.angle)),
          _falloff(radians(desc.falloff)),
          _direction(normalize(desc.direction)) {}
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
    [[nodiscard]] VSColor Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        Float3 w_un = p_ref.pos - _position;
        Float3 w = normalize(w_un);
        return _intensity / length_squared(w_un) * falloff(dot(_direction, w));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SpotLight)