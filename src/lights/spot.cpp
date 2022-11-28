//
// Created by Zero on 26/11/2022.
//

#include "base/light.h"

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
          _intensity(desc.intensity),
          _position(desc.position),
          _angle(radians(desc.angle)),
          _falloff(desc.falloff),
          _direction(normalize(desc.direction)) {}
    [[nodiscard]] float3 position() const noexcept override { return _position; }
    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        Float3 w_un = p_ref.pos - _position;
        Float3 w = normalize(w_un);
        float cos_angle = cos(_angle);
        return select(dot(_direction, w) > cos_angle, _intensity / length_squared(w_un), make_float3(0.f));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SpotLight)