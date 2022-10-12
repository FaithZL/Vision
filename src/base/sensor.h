//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"
#include "math/transform.h"
#include "filter.h"
#include "sample.h"
#include "descriptions/node_desc.h"

namespace vision {
using namespace ocarina;

class Sensor : public Node {
protected:
    Filter *_filter{};

public:
    explicit Sensor(SensorDesc *desc) : Node(desc->name) {}
    [[nodiscard]] virtual RaySample generate_ray(const SensorSample &ss) = 0;
};

struct CameraData {
    float3 position;
    float fov_y{20.f};
    float yaw{};
    float pitch{};
    float velocity{5.f};
    float sensitivity{1.f};
    float4x4 raster_to_screen{};
    float4x4 camera_to_screen{};
    float4x4 raster_to_camera{};
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

public:
    explicit Camera(SensorDesc *desc, CameraData *data)
        : Sensor(desc) { init(*data, desc); }
    void init(CameraData &data, const SensorDesc *desc) noexcept;
    [[nodiscard]] virtual float3 forward() const noexcept = 0;
    [[nodiscard]] virtual float3 up() const noexcept = 0;
    [[nodiscard]] virtual float3 right() const noexcept = 0;
    [[nodiscard]] virtual float3 position() const noexcept = 0;
    [[nodiscard]] virtual float yaw() const noexcept = 0;
    virtual void set_yaw(float yaw) noexcept = 0;
    virtual void update_yaw(float val) noexcept = 0;
    [[nodiscard]] virtual float pitch() const noexcept = 0;
    virtual void set_pitch(float pitch) noexcept = 0;
    virtual void update_pitch(float val) noexcept = 0;
    [[nodiscard]] virtual float fov_y() const noexcept = 0;
    virtual void set_fov_y(float val) noexcept = 0;
    virtual void update_fov_y(float val) noexcept = 0;
    virtual void update_device_data() noexcept = 0;
};

}// namespace vision