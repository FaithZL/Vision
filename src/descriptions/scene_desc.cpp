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
        mat_name_to_id[md.name] = i;
    }
}

void SceneDesc::init_shape_descs(const DataWrap &shapes) noexcept {
    for (uint i = 0; i < shapes.size(); ++i) {
        ShapeDesc sd;
        sd.init(shapes[i]);
        if (mat_name_to_id.contains(sd.material.name)) {
            sd.material.id = mat_name_to_id[sd.material.name];
            sd.mat_hash = material_descs[sd.material.id].hash();
        } else {
            sd.material.id = InvalidUI32;
            sd.mat_hash = InvalidUI64;
        }
        if (mediums_desc.has_mediums()) {
            if (!mediums_desc.global.empty() && sd.outside_medium.name.empty()) {
                sd.outside_medium.name = mediums_desc.global;
            }
            if (!mediums_desc.global.empty() && sd.inside_medium.name.empty()) {
                sd.inside_medium.name = mediums_desc.global;
            }
            sd.inside_medium.fill_id(mediums_desc.medium_name_to_id);
            sd.outside_medium.fill_id(mediums_desc.medium_name_to_id);
        }
        shape_descs.push_back(sd);
    }
}

void SceneDesc::init_medium_descs(const DataWrap &mediums) noexcept {
    if (!mediums.value("process", false)) {
        return;
    }
    mediums_desc.global = mediums.value("global", "");
    DataWrap lst = mediums.value("list", DataWrap());
    for (uint i = 0; i < lst.size(); ++i) {
        MediumDesc desc;
        desc.set_value("index", i);
        desc.init(lst[i]);
        mediums_desc.mediums.push_back(desc);
        mediums_desc.medium_name_to_id[desc.name] = i;
    }
}

void SceneDesc::process_materials() noexcept {
    // merge duplicate materials
    map<uint64_t, MaterialDesc> mat_map;
    map<uint64_t, uint> index_map;
    auto mats = ocarina::move(material_descs);
    bool has_no_material_light = false;
    for (const ShapeDesc &sd : shape_descs) {
        uint index = material_descs.size();
        if (sd.material.id == InvalidUI32) {
            if (sd.emission.valid()) {
                has_no_material_light = true;
            }
            continue;
        }
        MaterialDesc md = mats[sd.material.id];
        if (!mat_map.contains(sd.mat_hash)) {
            mat_map.insert(make_pair(sd.mat_hash, md));
            material_descs.push_back(md);
            index_map.insert(make_pair(sd.mat_hash, index));
        }
    }
    for (ShapeDesc &sd : shape_descs) {
        if (sd.material.id == InvalidUI32) {
            if (sd.emission.valid()) {
                sd.material.id = material_descs.size();
            }
            continue;
        }
        sd.material.id = index_map[sd.mat_hash];
    }
    if (has_no_material_light) {
        MaterialDesc md;
        md.sub_type = "null";
        material_descs.push_back(md);
    }
}

void SceneDesc::check_meshes() noexcept {
    for (auto sd : shape_descs) {
        if (!sd.material.valid() && !sd.inside_medium.valid() && !sd.outside_medium.valid()) {
            OC_WARNING("scene has no material mesh!");
        }
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
    sensor_desc.medium.fill_id(mediums_desc.medium_name_to_id);
    output_desc.init(data.value("output", DataWrap::object()));
    render_setting.init(data.value("render_setting", DataWrap::object()));
    denoiser_desc.init(data.value("denoiser", DataWrap::object()));
    pipeline_desc.init(data.value("pipeline", DataWrap::object()));
    process_materials();
    check_meshes();
}

SceneDesc SceneDesc::from_json(const fs::path &path) {
    SceneDesc scene_desc;
    scene_desc.scene_path = path.parent_path();
    DataWrap data = create_json_from_file(path);
    scene_desc.init(data);
    return scene_desc;
}

}// namespace vision