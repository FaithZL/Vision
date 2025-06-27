//
// Created by Zero on 10/09/2022.
//

#include "node_desc.h"
#include "parameter_set.h"
#include "core/macro_map.h"
#include "medium_scatter_data.h"
#include <sstream>
#include "math/transform.h"
#include "util/file_manager.h"

namespace vision {

string NodeDesc::parameter_string() const noexcept {
    return parameter_.data().dump();
}

string NodeDesc::file_name() const noexcept {
    return (*this)["fn"].as_string();
}

void NodeDesc::set_parameter(const ParameterSet &ps) noexcept {
    OC_ASSERT(ps.data().is_object());
    DataWrap data = ps.data();
    for (auto iter = data.begin(); iter != data.end(); ++iter) {
        parameter_.set_value(iter.key(), iter.value());
    }
}

void TransformDesc::init(const ParameterSet &ps) noexcept {
    if (ps.data().is_null()) {
        return;
    }
    sub_type = ps["type"].as_string("matrix4x4");
    ParameterSet param = ps.value("param");
    if (sub_type == "look_at") {
        float3 position = param["position"].as_float3(make_float3(0.f));
        float3 up = param["up"].as_float3(make_float3(0, 1, 0));
        float3 target_pos = param["target_pos"].as_float3(make_float3(0, 0, 1));
        mat = look_at<H>(position, target_pos, up);
    } else if (sub_type == "Euler") {
        float4x4 yaw_t = rotation_y<H>(param["yaw"].as_float(0.f), false);
        float4x4 roll_t = rotation_z<H>(param["roll"].as_float(0.f), false);
        float4x4 pitch_t = rotation_x<H>(param["pitch"].as_float(0.f), false);
        float4x4 tt = translation(param["position"].as_float3(make_float3(0.f)));
        mat = tt * pitch_t * roll_t * yaw_t;
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
    set_parameter(ps["param"]);
    ParameterSet param = parameter_;
    o2w.init(parameter_.data().value("transform", DataWrap::object()));
    if (parameter_.contains("emission")) {
        emission.init(parameter_["emission"]);
    }
}

bool ShapeDesc::operator==(const ShapeDesc &other) const noexcept {
    // todo hash
    return false;
}

void SamplerDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("independent");
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
}

void FilterDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("gaussian");
    set_parameter(ps.value("param", DataWrap::object()));
}

void SensorDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("thin_lens");
    set_parameter(ps.value("param"));
    transform_desc.init(parameter_.value("transform"));
    filter_desc.init(parameter_.value("filter"));
    film_desc.init(parameter_.value("film"));
    if (parameter_.contains("medium")) {
        medium.name = parameter_["medium"].as_string();
    }
}

void RadianceCollectorDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("normal");
}

void IntegratorDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("pt");
    set_parameter(ps.value("param"));
    ParameterSet denoiser_param = parameter_.value("denoiser");
    if (!denoiser_param.contains("type")) {
        denoiser_param.set_value("type", "svgf");
    }
    denoiser_desc.init(denoiser_param);
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

void SlotDesc::init(const ParameterSet &ps) noexcept {
    DataWrap data = ps.data();
    if (data.contains("channels")) {
        channels = ps["channels"].as_string();
        node.init(ps["node"]);
        output_key = ps.value("output_key").as_string();
    } else {
        node.init(ps);
    }
    if (dim() > 1 && node["value"].data().is_number()) {
        DataWrap value = DataWrap::array();
        for (int i = 0; i < dim(); ++i) {
            value.push_back(node["value"].data());
        }
        node.set_value("value", value);
    } else if (node["value"].data().is_array()) {
        //        OC_ASSERT(node["value"].data().size() == dim());
    } else if (data.is_number()) {
        // process scalar
        channels = "x";
        node.sub_type = "number";
        node.set_value("value", data);
    }
}

void MaterialDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    init_node_map(ps.value("node_tab"));
    sub_type = ps["type"].as_string("matte");
    if (sub_type == "mix" || sub_type == "add") {
        mat0 = make_shared<MaterialDesc>();
        mat0->init(ps["param"]["mat0"]);
        mat1 = make_shared<MaterialDesc>();
        mat1->init(ps["param"]["mat1"]);
        set_parameter(ps["param"]);
    } else {
        set_parameter(ps["param"]);
    }
}

void MediumDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("homogeneous");
    set_parameter(ps["param"]);
    string medium_name = parameter_["medium_name"].as_string();
    if (!medium_name.empty()) {
        auto [ss, sa] = detail::get_sigma(medium_name);
        sigma_s.init(DataWrap({ss.x, ss.y, ss.z}));
        sigma_a.init(DataWrap({sa.x, sa.y, sa.z}));
    } else {
        sigma_a.init(parameter_.value("sigma_a"));
        sigma_s.init(parameter_.value("sigma_s"));
    }
    scale.init(parameter_.value("scale"));
    g.init(parameter_.value("g"));
}

void LightDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("area");
    ParameterSet param = ps.value("param");
    set_parameter(ps.value("param"));
    color.init(param.value("color"));
    strength.init(param.value("scale"));
    o2w.init(param.value("o2w", DataWrap()));
}

void UVUnwrapperDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("xatlas");
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
}

void ToneMapperDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = "impl";
    construct_name = ps["type"].as_string("linear");
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
}

void RasterizerDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("cpu");
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
}

void ImporterDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
}

void PassDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
}

void FrameBufferDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("rt");
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
}

void PipelineDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("fixed");
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
    rasterizer_desc.init(param.value("rasterizer", DataWrap::object()));
    unwrapper_desc.init(param.value("uv_unwrapper", DataWrap::object()));
    frame_buffer_desc.init(param.value("frame_buffer", DataWrap::object()));
}

void DenoiserDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("oidn");
    ParameterSet param = ps.value("param", DataWrap::object());
    auto string = param.data().dump();
    set_parameter(param);
}

void GraphDesc::init(const vision::ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    init_node_map(ps.value("node_tab"));
    set_parameter(ps.value("param"));
}

void GraphDesc::init_node_map(const vision::ParameterSet &ps) noexcept {
    auto data = ps.data();
    for (const auto &[key, value] : data.items()) {
        ShaderNodeDesc snd;
        snd.init(value);
        auto str = to_string(value);
        node_map.insert(make_pair(key, snd));
    }
}

SlotDesc GraphDesc::slot(const std::string &key, const vision::DataWrap &data, vision::AttrTag tag) const noexcept {
    ShaderNodeDesc node{data, tag};
    node.name = key;
    uint size = data.is_number() ? 1 : data.size();
    SlotDesc slot_desc{node, size, tag};
    slot_desc.init(parameter_[key]);
    return slot_desc;
}

uint64_t ShaderNodeDesc::compute_hash() const noexcept {
    return hash64(NodeDesc::compute_hash(), parameter_string());
}

SP<SlotDesc> ShaderNodeDesc::slot(const std::string &key, AttrTag tag) const noexcept {
    auto data = parameter_[key].data();
    auto str = data.dump();
    ShaderNodeDesc node{data, tag};
    uint size = data.is_number() ? 1 : data.size();
    SP<SlotDesc> slot_desc = make_shared<SlotDesc>(node, size, tag);
    slot_desc->init(parameter_[key]);
    return slot_desc;
}

void ShaderNodeDesc::init(const ParameterSet &ps) noexcept {
    if (ps.data().is_null()) {
        return;
    }
    NodeDesc::init(ps);
    if (ps.data().is_array()) {
        sub_type = "number";
        parameter_.set_value("value", ps.data());
    } else if (ps.data().is_object() && !ps.contains("param")) {
        sub_type = ps.value("type", "image").as_string();
        if (sub_type == "image") {
            DataWrap json = DataWrap::object();
            json["fn"] = ps["fn"].as_string();
            json["color_space"] = ps["color_space"].data();
            parameter_.set_json(json);
        } else if (sub_type == "number") {
            DataWrap json = DataWrap::object();
            json["value"] = ps["value"].data();
            json["max"] = ps.value("max", 1.f).data();
            json["min"] = ps.value("min", 0.f).data();
            parameter_.set_json(json);
        }
    } else if (ps.data().is_number()) {
        parameter_.set_value("value", ps.as_float(1.f));
    } else {
        sub_type = ps["type"].as_string();
        set_parameter(ps["param"]);
    }
}

void LightSamplerDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("uniform");
    ParameterSet param = ps.value("param");
    set_parameter(param);
    for (const DataWrap &data : param["lights"].data()) {
        LightDesc light_desc;
        light_desc.init(data);
        light_descs.push_back(light_desc);
    }
}

void FilmDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("rgb");
    ParameterSet param = ps.value("param", DataWrap::object());
    set_parameter(param);
    tone_mapper.init(parameter_.value("tone_mapper", DataWrap::object()));
}

void WarperDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("alias_table");
}

void SpectrumDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    sub_type = ps["type"].as_string("srgb");
    set_parameter(ps.value("param", DataWrap::object()));
}

void OutputDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    if (ps.data().is_null()) {
        return;
    }
    spp = ps["spp"].as_uint(0u);
    save_exit = ps["save_exit"].as_uint(0u);
    fn = ps["fn"].as_string("output.png");
    denoise = ps["denoise"].as_bool(false);
}

void RenderSettingDesc::init(const ParameterSet &ps) noexcept {
    NodeDesc::init(ps);
    polymorphic_mode = static_cast<PolymorphicMode>(ps["polymorphic_mode"].as_uint(1));
    min_world_radius = ps["min_world_radius"].as_float(10);
    ray_offset_factor = ps["ray_offset_factor"].as_float(1.f);
}

}// namespace vision