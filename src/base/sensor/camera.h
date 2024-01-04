//
// Created by Zero on 2023/6/13.
//

#pragma once

#include "sensor.h"

namespace vision {

class Camera : public Sensor {
public:
    constexpr static float fov_max = 120.f;
    constexpr static float fov_min = 15.f;
    constexpr static float z_near = 0.01f;
    constexpr static float z_far = 1000.f;
    constexpr static float pitch_max = 80.f;

protected:
    float3 _position;
    float _yaw{};
    float _pitch{};
    float _velocity{5.f};
    float _sensitivity{1.f};
    float _fov_y{20.f};
    float4x4 _raster_to_screen{};
    float4x4 _camera_to_screen{};
    Serial<float> _tan_fov_y_over2{};
    Serial<float4x4> _c2w;
    Serial<float4x4> _prev_w2c;
    Serial<float4x4> _raster_to_camera{};
    Serial<float4x4> _prev_c2r{};
    /// previous position in world space
    Serial<float3> _prev_pos;

protected:
    void _update_raster() noexcept;
    void _update_resolution(uint2 res) noexcept;
    [[nodiscard]] virtual OCRay generate_ray_in_camera_space(const SensorSample &ss) const noexcept;

public:
    explicit Camera(const SensorDesc &desc);
    OC_SERIALIZABLE_FUNC(Sensor, _tan_fov_y_over2, _c2w, _prev_w2c,
                         _raster_to_camera, _prev_c2r, _prev_pos)
    void init(const SensorDesc &desc) noexcept;
    void update_mat(float4x4 m) noexcept;
    void set_mat(float4x4 m) noexcept;
    virtual void before_render() noexcept {}
    virtual void after_render() noexcept;
    void store_prev_data() noexcept;
    void set_sensitivity(float v) noexcept { _sensitivity = v; }
    [[nodiscard]] Float3 prev_raster_coord(Float3 pos);

    OC_MAKE_MEMBER_GETTER(sensitivity, )
    OC_MAKE_MEMBER_GETTER(position, )
    OC_MAKE_MEMBER_GETTER(yaw, )
    OC_MAKE_MEMBER_GETTER(velocity, )
    OC_MAKE_MEMBER_GETTER(pitch, )
    [[nodiscard]] Float3 device_position() const noexcept;
    [[nodiscard]] Float3 device_forward() const noexcept;
    [[nodiscard]] Float3 device_up() const noexcept;
    [[nodiscard]] Float3 device_right() const noexcept;
    void move(float3 delta) noexcept { _position += delta; }
    virtual void update_focal_distance(float val) noexcept {}
    [[nodiscard]] virtual float focal_distance() const noexcept { return 0; }
    virtual void update_lens_radius(float val) noexcept {}
    [[nodiscard]] virtual float lens_radius() const noexcept { return 0; }

    void set_resolution(ocarina::uint2 res) noexcept override;
    void set_yaw(decltype(_yaw) yaw) noexcept { _yaw = yaw; }
    void update_yaw(float val) noexcept { set_yaw(_yaw + val); }
    void set_pitch(float pitch) noexcept {
        if (pitch > pitch_max) {
            pitch = pitch_max;
        } else if (pitch < -pitch_max) {
            pitch = -pitch_max;
        }
        _pitch = pitch;
    }
    void update_pitch(float val) noexcept { set_pitch(pitch() + val); }
    [[nodiscard]] float fov_y() const noexcept { return _fov_y; }
    void set_fov_y(float new_fov_y) noexcept {
        if (new_fov_y > fov_max) {
            _fov_y = fov_max;
        } else if (new_fov_y < fov_min) {
            _fov_y = fov_min;
        } else {
            _fov_y = new_fov_y;
        }
        _tan_fov_y_over2 = tan(radians(_fov_y) * 0.5f);
        _update_raster();
    }
    void update_fov_y(float val) noexcept { set_fov_y(fov_y() + val); }
    virtual void update_device_data() noexcept;
    void prepare() noexcept override;
    [[nodiscard]] float4x4 camera_to_world() const noexcept;
    [[nodiscard]] float4x4 camera_to_world_rotation() const noexcept;
    [[nodiscard]] float3 forward() const noexcept;
    [[nodiscard]] float3 up() const noexcept;
    [[nodiscard]] float3 right() const noexcept;
    [[nodiscard]] RayState generate_ray(const SensorSample &ss) const noexcept override;
};

}// namespace vision