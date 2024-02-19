//
// Created by Zero on 2023/6/13.
//

#include "camera.h"
#include "base/mgr/pipeline.h"

namespace vision {

Camera::Camera(const SensorDesc &desc)
    : Sensor(desc) {
    init(desc);
    _update_resolution(_radiance_film->resolution());
}

void Camera::init(const SensorDesc &desc) noexcept {
    _velocity = desc["velocity"].as_float(10.f);
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
    Float3 p_film = make_float3(ss.p_film, 0.f);
    Float3 p_sensor = transform_point(*_raster_to_camera, p_film);
    OCRay ray = make_ray(make_float3(0.f),normalize(p_sensor));
    return ray;
}

void Camera::set_resolution(ocarina::uint2 res) noexcept {
    Sensor::set_resolution(res);
    _update_resolution(res);
}

void Camera::_update_resolution(uint2 res) noexcept {
    Box2f scrn = radiance_film()->screen_window();
    float2 span = scrn.span();
    float4x4 screen_to_raster = transform::scale(res.x, res.y, 1u) *
                                transform::scale(1 / span.x, 1 / -span.y, 1.f) *
                                transform::translation(-scrn.lower.x, -scrn.upper.y, 0.f);
    _raster_to_screen = inverse(screen_to_raster);
    _update_raster();
}

void Camera::_update_raster() noexcept {
    _camera_to_screen = transform::perspective<H>(fov_y(), z_near, z_far);
    _raster_to_camera = inverse(_camera_to_screen) * _raster_to_screen;
}

void Camera::update_mat(float4x4 m) noexcept {
    _pitch = degrees(std::atan2(m[1][2], m[1][1]));
    _yaw = degrees(std::atan2(m[2][0], m[0][0]));
    _position = make_float3(m[3]);
    _c2w = camera_to_world();
}

void Camera::set_mat(ocarina::float4x4 m) noexcept {
    _c2w = m;
    _position = make_float3(m[3]);
}

void Camera::after_render() noexcept {
    store_prev_data();
}

void Camera::store_prev_data() noexcept {
    _prev_c2r = inverse(_raster_to_camera.hv());
    _prev_w2c = inverse(_c2w.hv());
    _prev_pos = position();
}

Float3 Camera::prev_raster_coord(Float3 pos) {
    pos = transform_point(*_prev_w2c, pos);
    pos /= pos.z;
    Float3 ret = transform_point(*_prev_c2r, pos);
    return ret;
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
    store_prev_data();
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

Float4x4 Camera::device_c2w() const noexcept {
    return *_c2w;
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