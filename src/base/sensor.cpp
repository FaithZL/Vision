//
// Created by Zero on 11/10/2022.
//

#include "sensor.h"
#include "core/context.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;

Sensor::Sensor(const SensorDesc &desc)
    : Node(desc),
      _filter(desc.scene->load<Filter>(desc.filter_desc)),
      _film(desc.scene->load<Film>(desc.film_desc)) {}

void Sensor::prepare(RenderPipeline *rp) noexcept {
    _filter->prepare(rp);
    _film->prepare(rp);
}

void Camera::init(const SensorDesc &desc) noexcept {
    _data.emplace_back();
    _velocity = desc.velocity;
    _fov_y = desc.fov_y;
    update_mat(desc.transform_desc.mat);
}

RaySample Camera::generate_ray(const SensorSample &ss) const noexcept {
    uint2 res = _film->resolution();
    Var<Data> data = _data.read(0);
    Float2 p = (ss.p_film * 2.f - make_float2(res)) * data.tan_fov_y_over2 / float(res.y);
    Float3 dir = normalize(p.x * right() - p.y * up() + forward());
    RaySample ret;
    ret.ray = make_ray(position(), dir);
    ret.weight = 1.f;
    return ret;
}

void Camera::update_mat(float4x4 m) noexcept {
    float sy = sqrt(sqr(m[2][1]) + sqr(m[2][2]));
    _pitch = -degrees(std::atan2(m[1][2], m[1][1]));
    _yaw = degrees(-std::atan2(m[2][0], m[0][0]));
    _position = make_float3(m[3]);
    _data->c2w = camera_to_world();
}

void Camera::update_device_data() noexcept {
    _data.upload_immediately();
}

void Camera::prepare(RenderPipeline *rp) noexcept {
    Sensor::prepare(rp);
    _data.reset_device_buffer(rp->device(), 1);
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
    return _data->c2w[2].xyz();
}

float3 Camera::up() const noexcept {
    return _data->c2w[1].xyz();
}

float3 Camera::right() const noexcept {
    return _data->c2w[0].xyz();
}

}// namespace vision