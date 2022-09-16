//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "descriptions.h"
#include "shape.h"

namespace vision {
using namespace ocarina;
struct SceneDesc {
public:
    SensorDesc sensor_desc;
    SamplerDesc sampler_desc;
    LightSamplerDesc light_sampler_desc;
    IntegratorDesc integrator_desc;
    vector<MaterialDesc> material_desc;
    vector<TextureDesc> texture_desc;
    vector<ShapeDesc> shape_desc;
    vector<LightDesc> light_desc;

public:
    SceneDesc() = default;
    static unique_ptr<SceneDesc> from_json(const fs::path &path);
    void init(const DataWrap &data) noexcept;
};

}// namespace vision