//
// Created by Zero on 10/09/2022.
//

#include "descriptions.h"
#include "parameter_set.h"

namespace vision {

#define VISION_PARAMS_INITIAL(member_name) \
    member_name = param[#member_name].as<decltype(member_name)>(member_name);

void TransformDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("look_at");
    ParameterSet param = ps["param"];
    if (name == "look_at") {
        position = param["position"].as_float3();
        up = param["up"].as_float3();
        target_pos = param["target_pos"].as_float3();
    } else if (name == "yaw_pitch") {
        position = param["position"].as<float3>();
        yaw = param["yaw"].as_float();
        pitch = param["pitch"].as<decltype(pitch)>(pitch);
//        pitch = param["pitch"].as_float();
//        VISION_PARAMS_INITIAL(pitch)
    }
}
void ShapeDesc::init(const ParameterSet &ps) noexcept {
}
void SamplerDesc::init(const ParameterSet &ps) noexcept {
}
void FilterDesc::init(const ParameterSet &ps) noexcept {
}
void SensorDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("ThinLensCamera");
    ParameterSet param = ps["param"];
    velocity = param["velocity"].as_float(5.f);
    fov_y = param["fov_y"].as_float(20.f);
    transform_desc.init(param["transform"]);
    filter_desc.init(param["filter"]);
    film_desc.init(param["film"]);
}
void IntegratorDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("PTIntegrator");
    ParameterSet param = ps["param"];
    max_depth = param["max_depth"].as_uint(10);
    min_depth = param["min_depth"].as_uint(0);
    rr_threshold = param["rr_threshold"].as_float(1);
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
}
}// namespace vision