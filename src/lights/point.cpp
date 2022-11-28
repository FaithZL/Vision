//
// Created by Zero on 22/10/2022.
//

#include "base/light.h"

namespace vision {
class PointLight : public IPointLight {
private:
    float3 _intensity;
    float3 _position;

public:
    explicit PointLight(const LightDesc &desc)
        : IPointLight(desc),
          _intensity(desc.intensity),
          _position(desc.position) {}
    [[nodiscard]] float3 position() const noexcept override { return _position; }
    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        return _intensity / length_squared(p_ref.pos - _position);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PointLight)