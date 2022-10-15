//
// Created by Zero on 09/09/2022.
//

#include "base/lightsampler.h"

namespace vision {
class UniformLightSampler : public LightSampler {
public:
    explicit UniformLightSampler(const LightSamplerDesc *desc) : LightSampler(desc) {}
};
}

VS_MAKE_CLASS_CREATOR(vision::UniformLightSampler)