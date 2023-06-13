//
// Created by Zero on 22/10/2022.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/color/spectrum.h"

namespace vision {
class PointLight : public IPointLight {
private:
    Serial<float3> _position;

public:
    explicit PointLight(const LightDesc &desc)
        : IPointLight(desc),
          _position(desc["position"].as_float3()) {}
    OC_SERIALIZABLE_FUNC(_position)
    [[nodiscard]] float3 power() const noexcept override {
        return 4 * Pi * average();
    }
    [[nodiscard]] Float3 position() const noexcept override { return *_position; }
    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                             const LightEvalContext &p_light,
                             const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum value = _color.eval_illumination_spectrum(p_light.uv, swl).sample * scale();
        return value / length_squared(p_ref.pos - position());
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PointLight)