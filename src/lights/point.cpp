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
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PointLight)