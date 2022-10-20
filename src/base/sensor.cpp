//
// Created by Zero on 11/10/2022.
//

#include "sensor.h"
#include "core/context.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;

Sensor::Sensor(const SensorDesc *desc)
    : Node(desc),
      _filter(desc->scene->load_filter(&desc->filter_desc)),
      _film(desc->scene->load_film(&desc->film_desc)) {}

void Sensor::prepare(RenderPipeline *rp) noexcept {
    _filter->prepare(rp);
    _film->prepare(rp);
}

void Camera::init(const SensorDesc *desc) noexcept {
    _velocity = desc->velocity;
    _host_data.fov_y = desc->fov_y;
    update_mat(desc->transform_desc.mat);
}

void Camera::update_mat(float4x4 m) noexcept {
    float sy = sqrt(sqr(m[2][1]) + sqr(m[2][2]));
    _pitch = -degrees(std::atan2(m[1][2], m[1][1]));
    _yaw = degrees(-std::atan2(m[2][0], m[0][0]));
    _position = make_float3(m[3]);
    _host_data.c2w = camera_to_world();
}

void Camera::update_device_data() noexcept {

}

float4x4 Camera::camera_to_world_rotation() const noexcept {
    float4x4 horizontal = rotation_y<H>(yaw());
    float4x4 vertical = rotation_x<H>(-pitch());
    return horizontal * vertical;
}

float4x4 Camera::camera_to_world() const noexcept {
    float4x4 translate = translation(position());
    return translate * camera_to_world_rotation();
}

float3 Camera::forward() const noexcept {
    return _host_data.c2w[2].xyz();
}

float3 Camera::up() const noexcept {
    return _host_data.c2w[1].xyz();
}

float3 Camera::right() const noexcept {
    return _host_data.c2w[0].xyz();
}

}// namespace vision