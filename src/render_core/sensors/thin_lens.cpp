//
// Created by Zero on 09/09/2022.
//

#include "base/sensor/sensor.h"
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {
class ThinLensCamera : public Camera {
private:
    Serial<float> _focal_distance;
    Serial<float> _lens_radius;

public:
    explicit ThinLensCamera(const SensorDesc &desc)
        : Camera(desc),
          _focal_distance(desc["focal_distance"].as_float(5.f)),
          _lens_radius(desc["lens_radius"].as_float(0.f)) {
    }
    OC_SERIALIZABLE_FUNC(Camera, _focal_distance, _lens_radius)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        Camera::render_sub_UI(widgets);
        _changed |= widgets->input_float_limit("lens radius", &_lens_radius.hv(), 0, 0.5, 0.01, 0.01);
        _changed |= widgets->input_float_limit("focal distance", &_focal_distance.hv(), 0, 100, 0.1, 0.5);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void update_focal_distance(float val) noexcept override {
        float new_val = _focal_distance.hv() + val;
        if (new_val > 0.f) {
            _focal_distance = new_val;
        } else {
            _focal_distance = 0.1f;
        }
    }
    void update_lens_radius(float val) noexcept override {
        float new_val = _lens_radius.hv() + val;
        if (new_val >= 0.f) {
            _lens_radius = new_val;
        } else {
            _lens_radius = 0.f;
        }
    }
    [[nodiscard]] float focal_distance() const noexcept override {
        return _focal_distance.hv();
    }
    [[nodiscard]] float lens_radius() const noexcept override {
        return _lens_radius.hv();
    }
    [[nodiscard]] RayVar generate_ray_in_camera_space(const SensorSample &ss) const noexcept override {
        RayVar ray = Camera::generate_ray_in_camera_space(ss);
        Float2 p_lens = square_to_disk<D>(ss.p_lens) * *_lens_radius;
        Float ft = *_focal_distance / ray->direction().z;
        Float3 p_focus = ray->at(ft);
        ray->update_origin(make_float3(p_lens, 0.f));
        ray->update_direction(normalize(p_focus - ray->origin()));
        return ray;
    }
    [[nodiscard]] string to_string() noexcept override {
        return ocarina::format("camera yaw is {:.2f}, pitch is {:.2f}, fov is {:.1f}, focal distance is {:.2f}, "
                               "lens radius is {:.2f}, position is ({:.2f}, {:.2f}, {:.2f})",
                               yaw(), pitch(), fov_y(),
                               focal_distance(), lens_radius(),
                               position().x, position().y, position().z);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ThinLensCamera)