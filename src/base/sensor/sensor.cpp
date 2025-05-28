//
// Created by Zero on 2023/6/13.
//

#include "sensor.h"
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {

static float4x4 origin_matrix;

Sensor::Sensor(const SensorDesc &desc)
    : Photosensory(desc) {
    init(desc);
    update_resolution_(film_->resolution());
}

void Sensor::init(const SensorDesc &desc) noexcept {
    velocity_ = desc["velocity"].as_float(10.f);
    sensitivity_ = desc["sensitivity"].as_float(0.5f);
    set_fov_y(desc["fov_y"].as_float(20.f));
    update_mat(desc.transform_desc.mat);
    origin_matrix = desc.transform_desc.mat;
}

void Sensor::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    Photosensory::render_sub_UI(widgets);
    widgets->button_click("reset view", [&] {
        changed_ = true;
        update_mat(origin_matrix);
    });
    changed_ |= widgets->drag_float3("position", &position_, 0.05, 0, 0);
    bool changed_fov = widgets->drag_float("fov y", &fov_y_, 0.05, fov_min, fov_max);
    if (changed_fov) {
        set_fov_y(fov_y_);
    }
    changed_ |= changed_fov;
    changed_ |= widgets->drag_float("yaw", &yaw_, 0.05, 0, 0);
    changed_ |= widgets->drag_float("pitch", &pitch_, 0.05, -pitch_max, pitch_max);
    widgets->drag_float("velocity", &velocity_, 0.1, 0, 0);
}

RayState Sensor::generate_ray(const SensorSample &ss) const noexcept {
    RayVar ray = generate_ray_in_local_space(ss);
    Float4x4 c2w = *c2w_;
    ray = transform_ray(c2w, ray);
    return {.ray = ray, .ior = 1.f, .medium = *medium_id_};
}

RayVar Sensor::generate_ray_in_local_space(const vision::SensorSample &ss) const noexcept {
    Float3 p_film = make_float3(ss.p_film, 0.f);
    Float3 p_sensor = transform_point(*raster_to_sensor_, p_film);
    RayVar ray = make_ray(make_float3(0.f), normalize(p_sensor));
    return ray;
}

void Sensor::set_resolution(ocarina::uint2 res) noexcept {
    Photosensory::set_resolution(res);
    update_resolution_(res);
}

void Sensor::update_resolution_(uint2 res) noexcept {
    Box2f scrn = film()->screen_window();
    float2 span = scrn.span();
    float4x4 screen_to_raster = transform::scale(res.x, res.y, 1u) *
                                transform::scale(1 / span.x, 1 / -span.y, 1.f) *
                                transform::translation(-scrn.lower.x, -scrn.upper.y, 0.f);
    raster_to_screen_ = inverse(screen_to_raster);
    update_raster();
}

void Sensor::update_raster() noexcept {
    camera_to_screen_ = transform::perspective<H>(fov_y(), z_near, z_far);
    raster_to_sensor_ = inverse(camera_to_screen_) * raster_to_screen_;
}

void Sensor::update_mat(float4x4 m) noexcept {
    pitch_ = degrees(std::atan2(m[1][2], m[1][1]));
    yaw_ = degrees(std::atan2(m[2][0], m[0][0]));
    position_ = make_float3(m[3]);
    c2w_ = camera_to_world();
}

void Sensor::set_mat(ocarina::float4x4 m) noexcept {
    c2w_ = m;
    position_ = make_float3(m[3]);
}

void Sensor::update_resolution(ocarina::uint2 res) noexcept {
    film()->update_resolution(res);
    update_resolution_(res);
}

void Sensor::after_render() noexcept {
    store_prev_data();
}

void Sensor::store_prev_data() noexcept {
    prev_s2r_ = inverse(raster_to_sensor_.hv());
    prev_w2s_ = inverse(c2w_.hv());
    prev_pos_ = position();
}

Float3 Sensor::prev_raster_coord(Float3 pos) const noexcept {
    pos = transform_point(*prev_w2s_, pos);
    pos /= pos.z;
    Float3 ret = transform_point(*prev_s2r_, pos);
    return ret;
}

Float3 Sensor::raster_coord(ocarina::Float3 pos) const noexcept {
    Float4x4 w2c = inverse(device_c2w());
    pos = transform_point(w2c, pos);
    pos /= pos.z;
    Float4x4 c2r = inverse(raster_to_sensor_.dv());
    Float3 ret = transform_point(c2r, pos);
    return ret;
}

float3 Sensor::raster_coord(float3 pos) const noexcept {
    float4x4 w2c = inverse(c2w_.hv());
    pos = transform_point<H>(w2c, pos);
    pos /= pos.z;
    float4x4 c2r = inverse(raster_to_sensor_.hv());
    float3 ret = transform_point<H>(c2r, pos);
    return ret;
}

LineSegment Sensor::clipping(vision::LineSegment ls) const noexcept {
    float3 p0 = ls.p0;
    float3 p1 = ls.p1;
    float4x4 w2c = inverse(c2w_.hv());
    p0 = transform_point<H>(w2c, p0);
    p1 = transform_point<H>(w2c, p1);
    float3 sp0 = transform_point<H>(camera_to_screen_, p0);
    float3 sp1 = transform_point<H>(camera_to_screen_, p1);
    if (sp1.z > sp0.z) {
        float t = (sp1.z - 0.5) / (sp1.z - sp0.z);
        t = ocarina::clamp(t, 0.f, 1.f);
        ls.p0 = ocarina::lerp(t, ls.p1, ls.p0);
    } else {
        float t = (sp0.z - 0.5) / (sp0.z - sp1.z);
        t = ocarina::clamp(t, 0.f, 1.f);
        ls.p1 = ocarina::lerp(t, ls.p0, ls.p1);
    }
    return ls;
}

void Sensor::update_device_data() noexcept {
    c2w_ = camera_to_world();
    Photosensory::update_data();
    upload_immediately();
}

void Sensor::prepare() noexcept {
    Photosensory::prepare();
    prepare_data();
    upload_immediately();
    store_prev_data();
}

float4x4 Sensor::camera_to_world_rotation() const noexcept {
    float4x4 horizontal = rotation_y<H>(yaw());
    float4x4 vertical = rotation_x<H>(-pitch());
    return scale(1, 1, -1) * horizontal * vertical;
}

float4x4 Sensor::camera_to_world() const noexcept {
    float4x4 translate = translation(position_);
    return translate * camera_to_world_rotation();
}

float3 Sensor::forward() const noexcept {
    return c2w_.hv()[2].xyz();
}

float3 Sensor::up() const noexcept {
    return c2w_.hv()[1].xyz();
}

float3 Sensor::right() const noexcept {
    return c2w_.hv()[0].xyz();
}

Float4x4 Sensor::device_c2w() const noexcept {
    return *c2w_;
}

Float4x4 Sensor::device_w2c() const noexcept {
    return inverse(device_c2w());
}

Float Sensor::linear_depth(const Float3 &world_pos) const noexcept {
    Float3 c_pos = transform_point(device_w2c(), world_pos);
    return c_pos.z;
}

Float3 Sensor::device_forward() const noexcept {
    return (*c2w_)[2].xyz();
}

Float3 Sensor::device_up() const noexcept {
    return (*c2w_)[1].xyz();
}

Float3 Sensor::device_right() const noexcept {
    return (*c2w_)[0].xyz();
}

Float3 Sensor::device_position() const noexcept {
    return (*c2w_)[3].xyz();
}
}// namespace vision