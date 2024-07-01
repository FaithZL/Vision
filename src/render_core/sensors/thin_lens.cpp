//
// Created by Zero on 09/09/2022.
//

#include "base/sensor/sensor.h"
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {
class ThinLensCamera : public CameraImpl {
private:
    EncodedData<float> focal_distance_;
    EncodedData<float> lens_radius_;

public:
    explicit ThinLensCamera(const SensorDesc &desc)
        : CameraImpl(desc),
          focal_distance_(desc["focal_distance"].as_float(5.f)),
          lens_radius_(desc["lens_radius"].as_float(0.f)) {
    }
    OC_SERIALIZABLE_FUNC(CameraImpl, focal_distance_, lens_radius_)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        CameraImpl::render_sub_UI(widgets);
        changed_ |= widgets->input_float_limit("lens radius", &lens_radius_.hv(),
                                               0, 0.5, 0.01, 0.01);
        changed_ |= widgets->input_float_limit("focal distance", &focal_distance_.hv(),
                                               0, 100, 0.1, 0.5);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void update_focal_distance(float val) noexcept override {
        float new_val = focal_distance_.hv() + val;
        if (new_val > 0.f) {
            focal_distance_ = new_val;
        } else {
            focal_distance_ = 0.1f;
        }
    }
    void update_lens_radius(float val) noexcept override {
        float new_val = lens_radius_.hv() + val;
        if (new_val >= 0.f) {
            lens_radius_ = new_val;
        } else {
            lens_radius_ = 0.f;
        }
    }
    [[nodiscard]] float focal_distance() const noexcept override {
        return focal_distance_.hv();
    }
    [[nodiscard]] float lens_radius() const noexcept override {
        return lens_radius_.hv();
    }
    [[nodiscard]] RayVar generate_ray_in_camera_space(const SensorSample &ss) const noexcept override {
        RayVar ray = CameraImpl::generate_ray_in_camera_space(ss);
        Float2 p_lens = square_to_disk<D>(ss.p_lens) * *lens_radius_;
        Float ft = *focal_distance_ / ray->direction().z;
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