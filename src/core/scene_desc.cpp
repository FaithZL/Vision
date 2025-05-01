//
// Created by Zero on 06/09/2022.
//

#include "scene_desc.h"
#include "json_util.h"

namespace vision {

void SceneDesc::init_material_descs(const DataWrap &materials) noexcept {
    for (uint i = 0; i < materials.size(); ++i) {
        MaterialDesc md;
        md.init(materials[i]);
        material_descs.push_back(md);
    }
}

void SceneDesc::init_shape_descs(const DataWrap &shapes) noexcept {
    for (const auto &shape : shapes) {
        ShapeDesc sd;
        sd.init(shape);
        shape_descs.push_back(sd);
    }
}

void SceneDesc::init_medium_descs(const DataWrap &mediums) noexcept {
    mediums_desc.global = mediums.value("global", "");
    mediums_desc.process = mediums.value("process", false);
    DataWrap lst = mediums.value("list", DataWrap::object());
    for (const auto &elm : lst) {
        MediumDesc desc;
        desc.init(elm);
        mediums_desc.mediums.push_back(desc);
    }
}

void SceneDesc::init(const DataWrap &data) noexcept {
    integrator_desc.init(data.value("integrator", DataWrap::object()));
    spectrum_desc.init(data.value("spectrum", DataWrap::object()));
    light_sampler_desc.init(data.value("light_sampler", DataWrap::object()));
    sampler_desc.init(data.value("sampler", DataWrap::object()));
    warper_desc = WarperDesc("Warper");
    warper_desc.sub_type = "alias";
    init_material_descs(data.value("materials", DataWrap::object()));
    init_medium_descs(data.value("mediums", DataWrap::object()));
    init_shape_descs(data.value("shapes", DataWrap::object()));
    sensor_desc.init(data.value("camera", DataWrap::object()));
    sensor_desc.medium.name = mediums_desc.global;
    output_desc.init(data.value("output", DataWrap::object()));
    render_setting.init(data.value("render_setting", DataWrap::object()));
    denoiser_desc.init(data.value("denoiser", DataWrap::object()));
    pipeline_desc.init(data.value("pipeline", DataWrap::object()));
}

SceneDesc SceneDesc::from_json(const fs::path &path) {
    SceneDesc scene_desc;
    scene_desc.scene_path = path.parent_path();
    DataWrap data = create_json_from_file(path);
    scene_desc.init(data);
    return scene_desc;
}

}// namespace vision