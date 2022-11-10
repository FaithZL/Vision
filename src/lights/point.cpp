//
// Created by Zero on 22/10/2022.
//

#include "base/light.h"

namespace vision {
class PointLight : public Light {
private:
    float3 _intensity;
    float3 _position;

public:
    explicit PointLight(const LightDesc &desc)
        : Light(desc, LightType::DeltaPosition),
          _intensity(desc.intensity),
          _position(desc.position) {}

    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        return _intensity / length_squared(p_ref.pos - _position);
    }
    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        return 0.f;
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

VS_MAKE_CLASS_CREATOR(vision::PointLight)