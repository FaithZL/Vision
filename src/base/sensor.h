//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"
#include "math/transform.h"
#include "descriptions/node_desc.h"

namespace vision {
using namespace ocarina;

struct SensorSample {
    Float2 p_film;
    Float2 p_lens;
    Float time;
    Float filter_weight{1.f};
};

class Sensor : public Node {
protected:
    explicit Sensor(SensorDesc *desc) : Node(desc->name) {}
    [[nodiscard]] virtual float3 forward() const noexcept = 0;
    [[nodiscard]] virtual float3 up() const noexcept = 0;
    [[nodiscard]] virtual float3 right() const noexcept = 0;
    [[nodiscard]] virtual float3 position() const noexcept = 0;
};

class Camera : public Sensor {
public:
    constexpr static float fov_max = 120.f;
    constexpr static float fov_min = 15.f;

protected:
    constexpr static float z_near = 0.01f;
    constexpr static float z_far = 1000.f;
    constexpr static float pitch_max = 80.f;
    constexpr static float3 right_vec = make_float3(1, 0, 0);
    constexpr static float3 up_vec = make_float3(0, 1, 0);
    constexpr static float3 forward_vec = make_float3(0, 0, 1);
    float3 _position;
    float _fov_y{0};
    float _yaw{};
    float _pitch{};
    float _velocity{};
    float _sensitivity{1.f};
    Transform<float4x4> _raster_to_screen{};
    Transform<float4x4> _camera_to_screen{};
    Transform<float4x4> _raster_to_camera{};
};

}// namespace vision