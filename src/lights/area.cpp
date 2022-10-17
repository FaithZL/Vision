//
// Created by Zero on 09/09/2022.
//

#include "base/light.h"

namespace vision {

class AreaLight : public Light {
public:
    explicit AreaLight(const LightDesc *desc) : Light(desc, LightType::Area) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)