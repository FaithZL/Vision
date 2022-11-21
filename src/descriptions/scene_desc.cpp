//
// Created by Zero on 06/09/2022.
//

#include "scene_desc.h"

namespace vision {

namespace detail {
[[nodiscard]] std::string remove_cxx_comment(std::string source) {
    if (source.size() < 2) {
        return std::move(source);
    }

    const char *p0 = source.data();
    const char *p = p0;
    const char *p2 = p + 1;
    const char *pend = p + source.size();

    bool in_quote = false;
    bool in_sline_comment = false;
    bool in_mline_comment = false;
    bool all_whitespace = true;
    const char *pcontent = p;

    std::ostringstream ostrm;

    for (; p2 < pend; ++p, ++p2) {

        if (in_quote) {
            if (*p == '"')
                in_quote = false;
            continue;
        }

        if (in_sline_comment) {
            if (*p == '\n') {
                in_sline_comment = false;
                pcontent = p + (int)all_whitespace;
            } else if (*p == '\r' && *p2 == '\n') {
                in_sline_comment = false;
                pcontent = p + ((int)all_whitespace << 1);
                p = p2;
                ++p2;
            }
        } else {
            if (in_mline_comment) {
                if (*p == '*' && *p2 == '/') {
                    in_mline_comment = false;
                    pcontent = p + 2;
                    p = p2;
                    ++p2;
                }
            } else {
                // !in_quote && !in_sline_comment && !in_mline_comment
                if (*p == '"') {
                    in_quote = true;
                    all_whitespace = false;
                } else if (*p == '/') {
                    if (*p2 == '*') {
                        in_mline_comment = true;
                        ostrm.write(pcontent, p - pcontent);
                        p = p2;
                        ++p2;
                    } else if (*p2 == '/') {
                        in_sline_comment = true;
                        ostrm.write(pcontent, p - pcontent);
                        p = p2;
                        ++p2;
                    } else
                        all_whitespace = false;
                } else if (*p == '\n') {
                    all_whitespace = true;
                } else if (*p == '\r' && *p2 == '\n') {
                    all_whitespace = true;
                    p = p2;
                    ++p2;
                } else if (all_whitespace && *p != ' ')
                    all_whitespace = false;
            }
        }
    }

    if (!in_sline_comment && pcontent != pend) {
        if (pcontent == p0)
            return std::move(source);
        else
            ostrm.write(pcontent, pend - pcontent);
    }

    return ostrm.str();
}

[[nodiscard]] DataWrap create_json_from_file(const fs::path &fn) {
    std::ifstream fst;
    fst.open(fn.c_str());
    std::stringstream buffer;
    buffer << fst.rdbuf();
    std::string str = buffer.str();
    str = remove_cxx_comment(std::move(str));
    fst.close();
    if (fn.extension() == ".bson") {
        return DataWrap::from_bson(str);
    } else {
        return DataWrap::parse(str);
    }
}

}// namespace detail

void SceneDesc::init_material_descs(const DataWrap &materials) noexcept {
    for (uint i = 0; i < materials.size(); ++i) {
        MaterialDesc md;
        md.scene_path = scene_path;
        md.init(materials[i]);
        material_descs.push_back(md);
        mat_name_to_id[md.name] = i;
    }
}

void SceneDesc::init_shape_descs(const DataWrap &shapes) noexcept {
    for (uint i = 0; i < shapes.size(); ++i) {
        ShapeDesc shape_desc;
        shape_desc.index = i;
        shape_desc.scene_path = scene_path;
        shape_desc.init(shapes[i]);
        shape_desc.mat_id = mat_name_to_id[shape_desc.material_name];
        shape_desc.mat_hash = material_descs[shape_desc.mat_id].hash();
        shape_descs.push_back(shape_desc);
    }
}

void SceneDesc::process_materials() noexcept {
    map<uint64_t, MaterialDesc> mat_map;
    map<uint64_t, uint> index_map;
    auto mats = move(material_descs);
    for (const ShapeDesc &sd : shape_descs) {
        uint index = material_descs.size();
        MaterialDesc md = mats[sd.mat_id];
        if (!mat_map.contains(sd.mat_hash)) {
            mat_map.insert(make_pair(sd.mat_hash, md));
            material_descs.push_back(md);
            index_map.insert(make_pair(sd.mat_hash, index));
        }
    }
    for (ShapeDesc &sd : shape_descs) {
        sd.mat_id = index_map[sd.mat_hash];
    }
}

void SceneDesc::init(const DataWrap &data) noexcept {
    integrator_desc.init(data.value("integrator", DataWrap()));
    light_sampler_desc.init(data.value("light_sampler", DataWrap()));
    sampler_desc.init(data.value("sampler", DataWrap()));
    distribution_desc.init(data.value("warper", DataWrap()));
    sensor_desc.init(data.value("camera", DataWrap()));
    init_material_descs(data.value("materials", DataWrap()));
    init_shape_descs(data.value("shapes", DataWrap()));
    process_materials();
}

SceneDesc SceneDesc::from_json(const fs::path &path) {
    SceneDesc scene_desc;
    scene_desc.scene_path = path.parent_path();
    DataWrap data = detail::create_json_from_file(path);
    scene_desc.init(data);
    return scene_desc;
}


}// namespace vision