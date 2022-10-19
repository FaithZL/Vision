//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"
#include "math/transform.h"
#include "filter.h"
#include "film.h"
#include "sample.h"

namespace vision {
using namespace ocarina;

class Sensor : public Node {
public:
    using Desc = SensorDesc;

protected:
    Filter *_filter{};
    Film *_film{};

public:
    explicit Sensor(const SensorDesc *desc) : Node(desc) {}
    void set_filter(Filter *filter) noexcept { _filter = filter; }
    [[nodiscard]] Filter *filter() noexcept { return _filter; }
    void set_film(Film *film) noexcept { _film = film; }
    [[nodiscard]] Film *film() noexcept { return _film; }
    //    [[nodiscard]] virtual RaySample generate_ray(const SensorSample &ss) = 0;
};

}// namespace vision

namespace vision {
struct CameraData {
    float fov_y{20.f};
    float4x4 c2w;
};
}// namespace vision

OC_STRUCT(vision::CameraData, fov_y, c2w){};

namespace vision {

class Camera : public Sensor {
public:
    constexpr static float fov_max = 120.f;
    constexpr static float fov_min = 15.f;

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
    CameraData _host_data;

public:
    explicit Camera(const SensorDesc *desc)
        : Sensor(desc) { init(desc); }
    void init(const SensorDesc *desc) noexcept;
    void update_mat(float4x4 m) noexcept;
    [[nodiscard]] float3 position() const noexcept { return _position; }
    [[nodiscard]] float3 move(float3 delta) noexcept { _position += delta; }
    [[nodiscard]] float yaw() const noexcept { return _yaw; }
    [[nodiscard]] float velocity() const noexcept { return _velocity; }
    void set_yaw(float yaw) noexcept { _yaw = yaw; }
    void update_yaw(float val) noexcept { set_yaw(yaw() + val); }
    [[nodiscard]] float pitch() const noexcept { return _pitch; }
    void set_pitch(float pitch) noexcept { _pitch = pitch; }
    void update_pitch(float val) noexcept { set_pitch(pitch() + val); }
    [[nodiscard]] float fov_y() const noexcept { return _host_data.fov_y; }
    void set_fov_y(float new_fov_y) noexcept {
        if (new_fov_y > fov_max) {
            _host_data.fov_y = fov_max;
        } else if (new_fov_y < fov_min) {
            _host_data.fov_y = fov_min;
        } else {
            _host_data.fov_y = new_fov_y;
        }
    }
    void update_fov_y(float val) noexcept { set_fov_y(fov_y() + val); }
    virtual void update_device_data() noexcept;
    [[nodiscard]] float4x4 camera_to_world() noexcept;
    [[nodiscard]] float4x4 camera_to_world_rotation() noexcept;
    [[nodiscard]] float3 forward() const noexcept;
    [[nodiscard]] float3 up() const noexcept;
    [[nodiscard]] float3 right() const noexcept;
};

}// namespace vision