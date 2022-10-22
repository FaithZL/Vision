//
// Created by Zero on 22/10/2022.
//

#include "lightsampler.h"
#include "scene.h"

namespace vision {

LightSampler::LightSampler(const LightSamplerDesc &desc) : Node(desc) {
    for (const LightDesc &light_desc : desc.light_descs) {
        add_light(desc.scene->load<Light>(light_desc));
    }
}
}// namespace vision