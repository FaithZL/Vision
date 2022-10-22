//
// Created by Zero on 09/09/2022.
//

#include "base/light.h"

namespace vision {

class AreaLight : public Light {
private:
    uint _inst_idx{InvalidUI32};
    bool _two_sided{false};

public:
    explicit AreaLight(const LightDesc *desc)
        : Light(desc, LightType::Area),
        _two_sided{desc->two_sided}{}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)