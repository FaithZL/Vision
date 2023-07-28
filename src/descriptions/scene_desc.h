//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "node_desc.h"

namespace vision {
using namespace ocarina;
struct SceneDesc {
public:
    struct MediumsDesc {
        vector<MediumDesc> mediums;
        string global;
        NameID::map_ty medium_name_to_id;
        [[nodiscard]] bool has_mediums() const noexcept { return !mediums.empty(); }
    };

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
    ParameterSet ext_param;
    uint null_mat_index{InvalidUI32};

public:
    SceneDesc() = default;
    static SceneDesc from_json(const fs::path &path);
    void init_material_descs(const DataWrap &materials) noexcept;
    void init_shape_descs(const DataWrap &shapes) noexcept;
    void init_medium_descs(const DataWrap &mediums) noexcept;
    void init(const DataWrap &data) noexcept;
};

}// namespace vision