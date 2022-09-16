//
// Created by Zero on 10/09/2022.
//

#include "descriptions.h"
#include "parameter_set.h"
#include "core/macro_map.h"

namespace vision {

#define VISION_PARAMS_INITIAL(member_name) \
    this->member_name = param[#member_name].as<decltype(member_name)>(member_name);

#define VISION_PARAMS_LIST_INITIAL(...) MAP(VISION_PARAMS_INITIAL, ##__VA_ARGS__)

void TransformDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("look_at");
    ParameterSet param = ps["param"];
    if (name == "look_at") {
        VISION_PARAMS_LIST_INITIAL(position, up, target_pos)
    } else if (name == "yaw_pitch") {
        VISION_PARAMS_LIST_INITIAL(position, yaw, pitch)
    } else if (name == "trs") {
        VISION_PARAMS_LIST_INITIAL(t, r, s)
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
    VISION_PARAMS_LIST_INITIAL(resolution)
}

#undef VISION_PARAMS_INITIAL
#undef VISION_PARAMS_LIST_INITIAL

}// namespace vision