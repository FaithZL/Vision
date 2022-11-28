//
// Created by Zero on 27/11/2022.
//

#include "base/light.h"
#include "base/texture.h"

namespace vision {
class Projector : public IPointLight {
private:
    float4x4 _o2w;
    float _ratio;
    const Texture *_intensity{};
    float _angle_y;
    float _scale{1.f};

public:
    explicit Projector(const LightDesc &desc)
        : IPointLight(desc),
          _ratio(desc.ratio),
          _angle_y(desc.angle),
          _o2w(desc.o2w),
          _scale(desc.scale) {
    }
    [[nodiscard]] float3 position() const noexcept override { return _o2w[3].xyz(); }
    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        // todo
        return make_float3(0.f);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Projector)