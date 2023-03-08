//
// Created by Zero on 10/09/2022.
//

#include "node_desc.h"
#include "parameter_set.h"
#include "core/macro_map.h"
#include "medium_scatter_data.h"
#include <sstream>
#include "math/transform.h"

namespace vision {

#define VISION_PARAMS_INITIAL(member_name) \
    member_name = param[#member_name].as<decltype(member_name)>(member_name);

#define VISION_PARAMS_LIST_INITIAL(...) MAP(VISION_PARAMS_INITIAL, ##__VA_ARGS__)

void TransformDesc::init(const ParameterSet &ps) noexcept {
    if (ps.data().is_null()) {
        return;
    }
    sub_type = ps["type"].as_string("look_at");
    ParameterSet param = ps["param"];
    if (sub_type == "look_at") {
        float3 position = param["position"].as_float3(make_float3(0.f));
        float3 up = param["up"].as_float3(make_float3(0, 1, 0));
        float3 target_pos = param["target_pos"].as_float3(make_float3(0, 0, 1));
        mat = look_at<H>(position, target_pos, up);
    } else if (sub_type == "yaw_pitch") {
        float4x4 yaw_t = rotation_y<H>(param["yaw"].as_float(0.f), false);
        float4x4 pitch_t = rotation_x<H>(param["pitch"].as_float(0.f), false);
        float4x4 tt = translation(param["position"].as_float3(make_float3(0.f)));
        mat = tt * pitch_t * yaw_t;
    } else if (sub_type == "trs") {
        float3 t = param["t"].as_float3(make_float3(0.f));
        float4 r = param["r"].as_float4(make_float4(1, 0, 0, 0));
        float3 s = param["s"].as_float3(make_float3(1.f));
        mat = TRS<H>(t, r, s);
    } else if (sub_type == "matrix4x4") {
        mat = param["matrix4x4"].as_float4x4(make_float4x4(1.f));
    } else {
        OC_ERROR("transform type error ", sub_type);
    }
}

void ShapeDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string();
    name = ps["name"].as_string();
    _parameter = ps["param"];
    ParameterSet param = _parameter;
    o2w.init(_parameter.data().value("transform", DataWrap()));
    material.name = _parameter["material"].as_string("");
    if (_parameter.contains("medium")) {
        ParameterSet m(_parameter["medium"]);
        inside_medium.name = m["inside"].as_string();
        outside_medium.name = m["outside"].as_string();
    }
    if (_parameter.contains("emission")) {
        emission.inst_id = index;
        emission.scene_path = scene_path;
        emission.init(_parameter["emission"]);
    }
}

bool ShapeDesc::operator==(const ShapeDesc &other) const noexcept {
    // todo hash
    return false;
}

void SamplerDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("independent");
    _parameter = ps["param"];
}

void FilterDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("box");
    _parameter = ps["param"];
}

void SensorDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("thin_lens");
    _parameter = ps["param"];
    transform_desc.init(_parameter["transform"]);
    filter_desc.init(_parameter["filter"]);
    film_desc.init(_parameter["film"]);
    if (_parameter.contains("medium")) {
        medium.name = _parameter["medium"].as_string();
    }
}

void IntegratorDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("pt");
    _parameter = ps["param"];
}

namespace detail {

[[nodiscard]] pair<float3, float3> get_sigma(const string &name) {
    for (auto elm : SubsurfaceParameterTable) {
        if (elm.name == name) {
            return {elm.sigma_s, elm.sigma_a};
        }
    }
    MeasuredSS elm = SubsurfaceParameterTable[0];
    return {elm.sigma_s, elm.sigma_a};
}

}// namespace detail

void MaterialDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("matte");
    _parameter = ps["param"];
}

uint64_t MaterialDesc::_compute_hash() const noexcept {
    std::stringstream ss;
    ss << _parameter.data() << endl;
    return hash64(NodeDesc::_compute_hash(), ss.str());
}

void MediumDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("homogeneous");
    ParameterSet param = ps["param"];
    VISION_PARAMS_LIST_INITIAL(g, sigma_a, sigma_s, scale, medium_name)

    if (!medium_name.empty()) {
        auto [ss, sa] = detail::get_sigma(medium_name);
        sigma_s = ss;
        sigma_a = sa;
    }

    sigma_a *= scale;
    sigma_s *= scale;
}

void LightDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("area");
    ParameterSet param = ps["param"];

    if (sub_type == "area") {
        texture_desc.init(param["radiance"], scene_path);
        VISION_PARAMS_LIST_INITIAL(scale, two_sided)
    } else if (sub_type == "point" || sub_type == "spot") {
        VISION_PARAMS_LIST_INITIAL(position, intensity, scale, angle, falloff, direction)
        angle = clamp(angle, 1.f, 89.f);
        falloff = clamp(falloff, 0.f, angle);
        intensity *= scale;
    } else if (sub_type == "projector") {
        VISION_PARAMS_LIST_INITIAL(scale, angle, ratio)
        texture_desc.init(param["intensity"], scene_path);
        o2w.init(param.data().value("o2w", DataWrap()));
    } else if (sub_type == "environment") {
        VISION_PARAMS_LIST_INITIAL(scale)
        texture_desc.init(param["texture"], scene_path);
        o2w.init(param.data().value("o2w", DataWrap()));
    }
}

void ShaderNodeDesc::init(const ParameterSet &ps) noexcept {
    if (ps.data().is_null()) {
        return;
    }
    NodeDesc::init(ps);
    if (ps.data().is_array()) {
        sub_type = "constant";
        if (ps.data().size() == 2) {
            val = make_float4(ps.as_float2(), 0.f, 0.f);
        } else if (ps.data().size() == 3) {
            val = make_float4(ps.as_float3(), 0.f);
        } else {
            val = ps.as_float4();
        }
    } else if (ps.data().is_object()) {
        sub_type = "image";
        fn = (scene_path / ps["fn"].as_string()).string();
        color_space = ps["color_space"].as_string() == "linear" ?
                          ColorSpace::LINEAR :
                          ColorSpace::SRGB;
    } else if (ps.data().is_number()) {
        sub_type = "constant";
        val = make_float4(ps.as_float(1.f));
    }
}

void LightSamplerDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("uniform");
    ParameterSet param = ps["param"];
    VISION_PARAMS_LIST_INITIAL(env_prob)
    for (const DataWrap &data : param["lights"].data()) {
        LightDesc light_desc;
        light_desc.scene_path = scene_path;
        light_desc.init(data);
        light_descs.push_back(light_desc);
    }
}

void FilmDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("rgb");
    _parameter = ps["param"];
}

void WarperDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("alias_table");
}

void SpectrumDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("srgb");
    _parameter = ps.value("param", DataWrap::object());
}

void OutputDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    if (ps.data().is_null()) {
        return;
    }
    spp = ps["spp"].as_uint(0u);
    save_exit = ps["save_exit"].as_uint(0u);
    fn = (scene_path / ps["fn"].as_string()).string();
}

#undef VISION_PARAMS_INITIAL
#undef VISION_PARAMS_LIST_INITIAL

}// namespace vision