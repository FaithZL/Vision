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
public:
    using Desc = SensorDesc;

protected:
    Filter *_filter{};

public:
    explicit Sensor(const SensorDesc *desc) : Node(desc->name) {}
    //    [[nodiscard]] virtual RaySample generate_ray(const SensorSample &ss) = 0;
};

}// namespace vision

namespace vision {
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
}// namespace vision

OC_STRUCT(vision::CameraData, position, fov_y, yaw, pitch,
          velocity, sensitivity, raster_to_screen, camera_to_screen,
          raster_to_camera){};

namespace vision {

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

    CameraData _data;

public:
    explicit Camera(const SensorDesc *desc)
        : Sensor(desc) { init(desc); }
    void init(const SensorDesc *desc) noexcept;
    void update_mat(CameraData *data, float4x4 m) noexcept;
    [[nodiscard]] float3 forward() const noexcept;
    [[nodiscard]] float3 up() const noexcept;
    [[nodiscard]] float3 right() const noexcept;
    [[nodiscard]] float3 position() const noexcept;
    [[nodiscard]] float yaw() const noexcept;
    void set_yaw(float yaw) noexcept;
    void update_yaw(float val) noexcept;
    [[nodiscard]] float pitch() const noexcept;
    void set_pitch(float pitch) noexcept;
    void update_pitch(float val) noexcept;
    [[nodiscard]] float fov_y() const noexcept;
    void set_fov_y(float val) noexcept;
    void update_fov_y(float val) noexcept;
    virtual void update_device_data() noexcept;
    template<typename Scalar>
    [[nodiscard]] matrix4_t<Scalar> camera_to_world_rotation(const Scalar &yaw, const Scalar &pitch) {
        matrix4_t<Scalar> horizontal = rotation_y(yaw, false);
        matrix4_t<Scalar> vertical = rotation_x(-pitch, false);
        return horizontal * vertical;
    }

    template<typename Scalar>
    [[nodiscard]] vec3_t<Scalar> forward(const Scalar &yaw, const Scalar &pitch) {
        return transform_vector(camera_to_world_rotation(yaw, pitch), forward_vec);
    }

    template<typename Scalar>
    [[nodiscard]] vec3_t<Scalar> up(const Scalar &yaw, const Scalar &pitch) {
        return transform_vector(camera_to_world_rotation(yaw, pitch), up_vec);
    }

    template<typename Scalar>
    [[nodiscard]] vec3_t<Scalar> right(const Scalar &yaw, const Scalar &pitch) {
        return transform_vector(camera_to_world_rotation(yaw, pitch), right_vec);
    }

    template<typename Vec, typename Scalar>
    [[nodiscard]] matrix4_t<Scalar> camera_to_world(const Vec &pos, const Scalar &yaw, const Scalar &pitch) {
        matrix4_t<Vec> translate = translation(pos);
        return translate * camera_to_world_rotation(yaw, pitch);
    }
};

}// namespace vision