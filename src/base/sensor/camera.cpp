//
// Created by Zero on 2023/6/13.
//

#include "camera.h"
#include "base/mgr/pipeline.h"

namespace vision {

Camera::Camera(const SensorDesc &desc)
    : Sensor(desc), _data(pipeline()->resource_array()) {
    _data.resize(1);
    init(desc);
}

void Camera::init(const SensorDesc &desc) noexcept {
    _data.emplace_back();
    _velocity = desc["velocity"].as_float(5.f);
    _sensitivity = desc["sensitivity"].as_float(0.5f);
    set_fov_y(desc["fov_y"].as_float(20.f));
    update_mat(desc.transform_desc.mat);
}

RayState Camera::generate_ray(const SensorSample &ss) const noexcept {
    uint2 res = _radiance_film->resolution();
    Var<Data> data = _data.read(0);
    Float2 p = (ss.p_film * 2.f - make_float2(res)) * data.tan_fov_y_over2 / float(res.y);
    Float3 dir = normalize(p.x * device_right() - p.y * device_up() + device_forward());
    OCRay ray = make_ray(device_position(), dir);
    return {.ray = ray, .ior = 1.f, .medium = _medium};
}

void Camera::update_mat(float4x4 m) noexcept {
    _pitch = degrees(std::atan2(m[1][2], m[1][1]));
    _yaw = degrees(std::atan2(m[2][0], m[0][0]));
    _position = make_float3(m[3]);
    _data->c2w = camera_to_world();
}

void Camera::update_device_data() noexcept {
    _data->c2w = camera_to_world();
    _data.upload_immediately();
}

void Camera::prepare() noexcept {
    Sensor::prepare();
    _data.reset_device_buffer(pipeline()->device(), 1);
    _data.register_self();
}

float4x4 Camera::camera_to_world_rotation() const noexcept {
    float4x4 horizontal = rotation_y<H>(yaw().hv());
    float4x4 vertical = rotation_x<H>(-pitch().hv());
    return scale(1, 1, -1) * horizontal * vertical;
}

float4x4 Camera::camera_to_world() const noexcept {
    float4x4 translate = translation(position().hv());
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

Float3 Camera::device_forward() const noexcept {
    Var<Data> data = _data.read(0);
    return data->c2w[2].xyz();
}

Float3 Camera::device_up() const noexcept {
    Var<Data> data = _data.read(0);
    return data->c2w[1].xyz();
}

Float3 Camera::device_right() const noexcept {
    Var<Data> data = _data.read(0);
    return data->c2w[0].xyz();
}

Float3 Camera::device_position() const noexcept {
    Var<Data> data = _data.read(0);
    return data->c2w[3].xyz();
}
}// namespace vision