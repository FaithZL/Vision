//
// Created by Zero on 10/09/2022.
//

#include "descriptions.h"
#include "parameter_set.h"
#include "core/macro_map.h"
#include "math/transform.h"

namespace vision {

#define VISION_PARAMS_INITIAL(member_name) \
    member_name = param[#member_name].as<decltype(member_name)>(member_name);

#define VISION_PARAMS_LIST_INITIAL(...) MAP(VISION_PARAMS_INITIAL, ##__VA_ARGS__)

void TransformDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("look_at");
    ParameterSet param = ps["param"];
    if (name == "look_at") {
        float3 position = param["position"].as_float3(make_float3(0.f));
        float3 up = param["up"].as_float3(make_float3(0, 1, 0));
        float3 target_pos = param["target_pos"].as_float3(make_float3(0, 0, 1));
        mat = look_at(position, target_pos, up);
    } else if (name == "yaw_pitch") {
        auto yaw_t = rotation_y(param["yaw"].as_float(0.f), false);
        auto pitch_t = rotation_x(param["pitch"].as_float(0.f), false);
        auto tt = translation(param["position"].as_float3(make_float3(0.f)));
        mat = tt * pitch_t * yaw_t;
    } else if (name == "trs") {
        float3 t = param["t"].as_float3(make_float3(0.f));
        float4 r = param["r"].as_float4(make_float4(1, 0, 0, 0));
        float3 s = param["s"].as_float3(make_float3(1.f));
        mat = TRS(t, r, s);
    } else if (name == "mat4x4"){
        mat = param["mat4x4"].as_float4x4(make_float4x4(1.f));
    } else {
        OC_ERROR("transform type error ", name);
    }
}

void ShapeDesc::init(const ParameterSet &ps) noexcept {
}

void SamplerDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("IndependentSampler");
    ParameterSet param = ps["param"];
    VISION_PARAMS_INITIAL(spp)
}

void FilterDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("BoxFilter");
    ParameterSet param = ps["param"];
    VISION_PARAMS_INITIAL(radius)
}

void SensorDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("ThinLensCamera");
    ParameterSet param = ps["param"];
    VISION_PARAMS_LIST_INITIAL(velocity, fov_y, focal_distance, lens_radius)
    transform_desc.init(param["transform"]);
    filter_desc.init(param["filter"]);
    film_desc.init(param["film"]);
}
void IntegratorDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("PTIntegrator");
    ParameterSet param = ps["param"];
    VISION_PARAMS_LIST_INITIAL(max_depth, min_depth, rr_threshold)
}
void MaterialDesc::init(const ParameterSet &ps) noexcept {
}
void LightDesc::init(const ParameterSet &ps) noexcept {
}
void TextureDesc::init(const ParameterSet &ps) noexcept {
}
void LightSamplerDesc::init(const ParameterSet &ps) noexcept {
}
void FilmDesc::init(const ParameterSet &ps) noexcept {
    ParameterSet param = ps["param"];
    VISION_PARAMS_LIST_INITIAL(resolution)
}

#undef VISION_PARAMS_INITIAL
#undef VISION_PARAMS_LIST_INITIAL

}// namespace vision