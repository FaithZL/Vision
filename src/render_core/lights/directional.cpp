//
// Created by Zero on 2023/8/7.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/color/spectrum.h"

namespace vision {

class DirectionalLight : public Light {
private:
    Serial<float3> _direction;

public:
    explicit DirectionalLight(const LightDesc &desc)
        : Light(desc, LightType::DeltaDirection) {}
};

}// namespace vision