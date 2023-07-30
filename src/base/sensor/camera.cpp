//
// Created by Zero on 2023/6/13.
//

#include "camera.h"
#include "base/mgr/pipeline.h"

namespace vision {

Camera::Camera(const SensorDesc &desc)
    : Sensor(desc) {
    init(desc);
}

void Camera::init(const SensorDesc &desc) noexcept {
    _velocity = desc["velocity"].as_float(5.f);
    _sensitivity = desc["sensitivity"].as_float(0.5f);
    set_fov_y(desc["fov_y"].as_float(20.f));
    update_mat(desc.transform_desc.mat);
}

RayState Camera::generate_ray(const SensorSample &ss) const noexcept {
    OCRay ray = generate_ray_in_camera_space(ss);
    Float4x4 c2w = *_c2w;
    ray = transform_ray(c2w, ray);
    return {.ray = ray, .ior = 1.f, .medium = *_medium_id};
}

OCRay Camera::generate_ray_in_camera_space(const vision::SensorSample &ss) const noexcept {
    uint2 res = _radiance_film->resolution();
    Float2 p = (ss.p_film * 2.f - make_float2(res)) * *_tan_fov_y_over2 / float(res.y);
    Float3 dir = normalize(make_float3(p.x, -p.y, 1.f));
    OCRay ray = make_ray(make_float3(0.f), dir);
    return ray;
}

void Camera::update_mat(float4x4 m) noexcept {
    _pitch = degrees(std::atan2(m[1][2], m[1][1]));
    _yaw = degrees(std::atan2(m[2][0], m[0][0]));
    _position = make_float3(m[3]);
    _c2w = camera_to_world();
}

void Camera::update_device_data() noexcept {
    _c2w = camera_to_world();
    Sensor::update_data();
    upload_immediately();
}

void Camera::prepare() noexcept {
    Sensor::prepare();
    prepare_data();
    upload_immediately();
}

float4x4 Camera::camera_to_world_rotation() const noexcept {
    float4x4 horizontal = rotation_y<H>(yaw());
    float4x4 vertical = rotation_x<H>(-pitch());
    return scale(1, 1, -1) * horizontal * vertical;
}

float4x4 Camera::camera_to_world() const noexcept {
    float4x4 translate = translation(_position);
    return translate * camera_to_world_rotation();
}

float3 Camera::forward() const noexcept {
    return _c2w.hv()[2].xyz();
}

float3 Camera::up() const noexcept {
    return _c2w.hv()[1].xyz();
}

float3 Camera::right() const noexcept {
    return _c2w.hv()[0].xyz();
}

Float3 Camera::device_forward() const noexcept {
    return (*_c2w)[2].xyz();
}

Float3 Camera::device_up() const noexcept {
    return (*_c2w)[1].xyz();
}

Float3 Camera::device_right() const noexcept {
    return (*_c2w)[0].xyz();
}

Float3 Camera::device_position() const noexcept {
    return (*_c2w)[3].xyz();
}
}// namespace vision