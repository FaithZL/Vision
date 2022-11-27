//
// Created by Zero on 26/11/2022.
//

#include "base/light.h"

namespace vision {
class SpotLight : public Light {
private:
    float3 _intensity;
    float3 _position;
    float3 _direction;
    float _angle;
    // falloff angle range
    float _falloff;

public:
    explicit SpotLight(const LightDesc &desc)
        : Light(desc, LightType::DeltaPosition),
          _intensity(desc.intensity),
          _position(desc.position) {}

    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        return _intensity / length_squared(p_ref.pos - _position);
    }
    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        // using -1 for delta position light
        return -1.f;
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u) const noexcept override {
        LightSample ret;
        LightEvalContext p_light;
        p_light.pos = _position;
        ret.eval = evaluate(p_ref, p_light);
        Float3 wi_un = _position - p_ref.pos;
        ret.p_light = p_light.pos;
        return ret;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SpotLight)