//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"
#include "sample.h"
#include "math/transform.h"
#include "filter.h"
#include "film.h"

namespace vision {
using namespace ocarina;

struct SensorSample {
    Float2 p_film;
    Float2 p_lens;
    Float time;
    Float filter_weight{1.f};
};

class Sensor : public Node {
public:
    using Desc = SensorDesc;

protected:
    Filter *_filter{};
    Film *_film{};
    uchar _medium{InvalidUI8};

public:
    explicit Sensor(const SensorDesc &desc);
    void prepare() noexcept override;
    [[nodiscard]] Filter *filter() noexcept { return _filter; }
    [[nodiscard]] auto film() noexcept { return _film; }
    [[nodiscard]] auto film() const noexcept { return _film; }
    [[nodiscard]] uint2 resolution() noexcept { return _film->resolution(); }
    [[nodiscard]] virtual RayState generate_ray(const SensorSample &ss) const noexcept = 0;
};

}// namespace vision

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
    constexpr static float3 right_vec = make_float3(1, 0, 0);
    constexpr static float3 up_vec = make_float3(0, 1, 0);
    constexpr static float3 forward_vec = make_float3(0, 0, 1);

    float3 _position;
    float _yaw{};
    float _pitch{};
    float _velocity{5.f};
    float _sensitivity{1.f};
    float _fov_y{20.f};
    ManagedWrapper<Data> _data;

public:
    [[nodiscard]] Float3 device_forward() const noexcept;
    [[nodiscard]] Float3 device_up() const noexcept;
    [[nodiscard]] Float3 device_right() const noexcept;
    [[nodiscard]] Float3 device_position() const noexcept;

public:
    explicit Camera(const SensorDesc &desc);
    void init(const SensorDesc &desc) noexcept;
    void update_mat(float4x4 m) noexcept;
    void set_sensitivity(float v) noexcept { _sensitivity = v; }
    [[nodiscard]] float sensitivity() const noexcept { return _sensitivity; }
    [[nodiscard]] float3 position() const noexcept { return _position; }
    void move(float3 delta) noexcept { _position += delta; }
    [[nodiscard]] float yaw() const noexcept { return _yaw; }
    [[nodiscard]] float velocity() const noexcept { return _velocity; }
    void set_yaw(float yaw) noexcept { _yaw = yaw; }
    void update_yaw(float val) noexcept { set_yaw(yaw() + val); }
    [[nodiscard]] float pitch() const noexcept { return _pitch; }
    void set_pitch(float pitch) noexcept { _pitch = pitch; }
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
        _data->tan_fov_y_over2 = tan(radians(_fov_y) * 0.5f);
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