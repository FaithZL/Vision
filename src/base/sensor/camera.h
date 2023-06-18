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

    struct Data {
        float tan_fov_y_over2{};
        float4x4 c2w;
    };

protected:
    constexpr static float pitch_max = 80.f;

    Serial<float3> _position;
    Serial<float> _yaw{};
    Serial<float> _pitch{};
    Serial<float> _velocity{5.f};
    Serial<float> _sensitivity{1.f};
    float _fov_y{20.f};
    Serial<float> _tan_fov_y_over2{};
    Serial<float4x4> _c2w;

public:
    [[nodiscard]] Float3 device_forward() const noexcept;
    [[nodiscard]] Float3 device_up() const noexcept;
    [[nodiscard]] Float3 device_right() const noexcept;
    [[nodiscard]] Float3 device_position() const noexcept;

public:
    explicit Camera(const SensorDesc &desc);
    OC_SERIALIZABLE_FUNC(Sensor, _position, _yaw, _pitch, _velocity,
                         _sensitivity, _tan_fov_y_over2, _c2w)
    void init(const SensorDesc &desc) noexcept;
    void update_mat(float4x4 m) noexcept;
    void set_sensitivity(float v) noexcept { _sensitivity = v; }

#define VS_MAKE_MEMBER_GETTER(attr)                                     \
    [[nodiscard]] const auto &attr() const noexcept { return _##attr; } \
    [[nodiscard]] auto &attr() noexcept { return _##attr; }
    VS_MAKE_MEMBER_GETTER(sensitivity)
    VS_MAKE_MEMBER_GETTER(position)
    VS_MAKE_MEMBER_GETTER(yaw)
    VS_MAKE_MEMBER_GETTER(velocity)
    VS_MAKE_MEMBER_GETTER(pitch)
#undef VS_MAKE_MEMBER_GETTER

    void move(float3 delta) noexcept { _position.hv() += delta; }
    void set_yaw(float yaw) noexcept { _yaw = yaw; }
    void update_yaw(float val) noexcept { set_yaw(yaw().hv() + val); }
    void set_pitch(float pitch) noexcept {
        if (pitch > pitch_max) {
            pitch = pitch_max;
        } else if (pitch < -pitch_max) {
            pitch = -pitch_max;
        }
        _pitch = pitch;
    }
    void update_pitch(float val) noexcept { set_pitch(pitch().hv() + val); }
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

OC_STRUCT(vision::Camera::Data, tan_fov_y_over2, c2w){};