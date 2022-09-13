//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "descriptions.h"

namespace vision {
using namespace ocarina;
struct SceneDesc {
public:
    SensorDesc sensor_desc;
    SamplerDesc sampler_desc;
    FilterDesc filter_desc;
    LightSamplerDesc light_sampler_desc;
    IntegratorDesc integrator_desc;
    vector<unique_ptr<MaterialDesc>> material_desc;
    vector<unique_ptr<TextureDesc>> texture_desc;

public:
    SceneDesc() = default;
    static unique_ptr<SceneDesc> from_json(const fs::path &path);
};

}// namespace vision