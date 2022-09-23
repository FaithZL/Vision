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
    vector<MaterialDesc> material_descs;
    vector<ShapeDesc> shape_descs;
    vector<LightDesc> light_descs;

public:
    SceneDesc() = default;
    static unique_ptr<SceneDesc> from_json(const fs::path &path);
    void init_material_descs(const DataWrap &materials) noexcept;
    void init_shape_descs(const DataWrap &shapes) noexcept;
    void init_light_descs(const DataWrap &lights) noexcept;
    void init(const DataWrap &data) noexcept;
};

}// namespace vision