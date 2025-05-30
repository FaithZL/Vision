//
// Created by Zero on 09/09/2022.
//

#include "base/sensor/photosensory.h"
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {
class ThinLensCamera : public Sensor {
private:
    EncodedData<float> focal_distance_;
    EncodedData<float> lens_radius_;

public:
    ThinLensCamera() = default;
    explicit ThinLensCamera(const SensorDesc &desc)
        : Sensor(desc),
          focal_distance_(desc["focal_distance"].as_float(5.f)),
          lens_radius_(desc["lens_radius"].as_float(0.f)) {
    }
    OC_ENCODABLE_FUNC(Sensor, focal_distance_, lens_radius_)
    VS_HOTFIX_MAKE_RESTORE(Sensor, focal_distance_, lens_radius_)
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        Sensor::render_sub_UI(widgets);
        changed_ |= widgets->drag_float("lens radius", addressof(lens_radius_.hv()),
                                        0.005, 0, 0.5);
        changed_ |= widgets->drag_float("focal distance", addressof(focal_distance_.hv()),
                                        0.05, 0, 10000);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] float focal_distance() const noexcept { return focal_distance_.hv(); }
    [[nodiscard]] float lens_radius() const noexcept { return lens_radius_.hv(); }
    [[nodiscard]] RayVar generate_ray_in_local_space(const SensorSample &ss) const noexcept override {
        RayVar ray = Sensor::generate_ray_in_local_space(ss);
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
VS_MAKE_CLASS_CREATOR_HOTFIX(vision, ThinLensCamera)