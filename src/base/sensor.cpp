//
// Created by Zero on 11/10/2022.
//

#include "sensor.h"

namespace vision {
using namespace ocarina;
void Camera::init(CameraData *data, const SensorDesc *desc) noexcept {
    data->velocity = desc->velocity;
    data->fov_y = desc->fov_y;
    update_mat(data, desc->transform_desc.mat);
}

void Camera::update_mat(CameraData *data, float4x4 m) noexcept {
    float sy = sqrt(sqr(m[2][1]) + sqr(m[2][2]));
    data->pitch = -degrees(std::atan2(m[1][2], m[1][1]));
    data->yaw = degrees(-std::atan2(m[2][0], m[0][0]));
    data->position = make_float3(m[3]);
}

}// namespace vision