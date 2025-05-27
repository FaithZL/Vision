//
// Created by Zero on 2023/6/13.
//

#pragma once

#include "photosensory.h"

namespace vision {

class Sensor : public Photosensory {
public:
    constexpr static float fov_max = 120.f;
    constexpr static float fov_min = 15.f;
    constexpr static float z_near = 0.01f;
    constexpr static float z_far = 1000.f;
    constexpr static float pitch_max = 80.f;

protected:
    float3 position_;
    float yaw_{};
    float pitch_{};
    float velocity_{5.f};
    float sensitivity_{1.f};
    float fov_y_{20.f};
    float4x4 raster_to_screen_{};
    float4x4 camera_to_screen_{};
    EncodedData<float> tan_fov_y_over_2_{};
    EncodedData<float4x4> c2w_;
    EncodedData<float4x4> prev_w2s_;
    EncodedData<float4x4> raster_to_sensor_{};
    EncodedData<float4x4> prev_s2r_{};
    /// previous position in world space
    EncodedData<float3> prev_pos_;

protected:
    void update_raster() noexcept;
    void update_resolution_(uint2 res) noexcept;
    [[nodiscard]] virtual RayVar generate_ray_in_local_space(const SensorSample &ss) const noexcept;

public:
    Sensor() = default;
    explicit Sensor(const SensorDesc &desc);
    OC_ENCODABLE_FUNC(Photosensory, tan_fov_y_over_2_, c2w_, prev_w2s_,
                      raster_to_sensor_, prev_s2r_, prev_pos_)
    VS_HOTFIX_MAKE_RESTORE(Photosensory, position_, yaw_,pitch_,velocity_,sensitivity_,
                           fov_y_,raster_to_screen_,camera_to_screen_,tan_fov_y_over_2_,
                           c2w_,prev_w2s_, raster_to_sensor_,prev_s2r_,prev_pos_)
    void init(const SensorDesc &desc) noexcept;
    void update_mat(float4x4 m) noexcept;
    void set_mat(float4x4 m) noexcept;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    virtual void before_render() noexcept {}
    virtual void after_render() noexcept;
    void store_prev_data() noexcept;
    void update_resolution(uint2 res) noexcept;
    void set_sensitivity(float v) noexcept { sensitivity_ = v; }
    [[nodiscard]] Float3 prev_raster_coord(Float3 pos) const noexcept;
    [[nodiscard]] Float3 raster_coord(Float3 pos) const noexcept;
    [[nodiscard]] float3 raster_coord(float3 pos) const noexcept;
    [[nodiscard]] LineSegment clipping(LineSegment ls) const noexcept;
    [[nodiscard]] Float3 prev_device_position() const noexcept { return *prev_pos_; }
    OC_MAKE_MEMBER_GETTER(sensitivity, )
    OC_MAKE_MEMBER_GETTER(position, )
    OC_MAKE_MEMBER_GETTER(yaw, )
    OC_MAKE_MEMBER_GETTER(velocity, )
    OC_MAKE_MEMBER_GETTER(pitch, )
    [[nodiscard]] Float3 device_position() const noexcept;
    [[nodiscard]] Float3 device_forward() const noexcept;
    [[nodiscard]] Float3 device_up() const noexcept;
    [[nodiscard]] Float3 device_right() const noexcept;
    [[nodiscard]] Float4x4 device_c2w() const noexcept;
    [[nodiscard]] Float linear_depth(const Float3 &world_pos) const noexcept;
    void move(float3 delta) noexcept { position_ += delta; }

    void set_resolution(ocarina::uint2 res) noexcept override;
    void set_yaw(decltype(yaw_) yaw) noexcept { yaw_ = yaw; }
    void update_yaw(float val) noexcept { set_yaw(yaw_ + val); }
    void set_pitch(float pitch) noexcept {
        if (pitch > pitch_max) {
            pitch = pitch_max;
        } else if (pitch < -pitch_max) {
            pitch = -pitch_max;
        }
        pitch_ = pitch;
    }
    void update_pitch(float val) noexcept { set_pitch(pitch() + val); }
    [[nodiscard]] float fov_y() const noexcept { return fov_y_; }
    void set_fov_y(float new_fov_y) noexcept {
        if (new_fov_y > fov_max) {
            fov_y_ = fov_max;
        } else if (new_fov_y < fov_min) {
            fov_y_ = fov_min;
        } else {
            fov_y_ = new_fov_y;
        }
        tan_fov_y_over_2_ = tan(radians(fov_y_) * 0.5f);
        update_raster();
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

using TSensor = TObject<Sensor>;

}// namespace vision