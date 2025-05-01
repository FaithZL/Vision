//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "math/basic_types.h"
#include "core/stl.h"
#include "node_desc.h"

namespace vision {
using namespace ocarina;

struct MediumsDesc {
    vector<MediumDesc> mediums;
    bool process{false};
    string global;
};

struct SceneDesc {
public:
    SensorDesc sensor_desc;
    SamplerDesc sampler_desc;
    SpectrumDesc spectrum_desc;
    LightSamplerDesc light_sampler_desc;
    IntegratorDesc integrator_desc;
    WarperDesc warper_desc;
    vector<MaterialDesc> material_descs;
    vector<ShapeDesc> shape_descs;
    OutputDesc output_desc;
    PipelineDesc pipeline_desc;
    fs::path scene_path;
    MediumsDesc mediums_desc;
    DenoiserDesc denoiser_desc;
    RenderSettingDesc render_setting;

public:
    SceneDesc() = default;
    static SceneDesc from_json(const fs::path &path);
    void init_material_descs(const DataWrap &materials) noexcept;
    void init_shape_descs(const DataWrap &shapes) noexcept;
    void init_medium_descs(const DataWrap &mediums) noexcept;
    void init(const DataWrap &data) noexcept;
};

}// namespace vision