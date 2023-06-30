//
// Created by Zero on 09/09/2022.
//

#include "base/sensor/sensor.h"
#include "base/mgr/pipeline.h"

namespace vision {
class ThinLensCamera : public Camera {
public:
    struct LensData {
        float focal_distance{};
        float lens_radius{0.f};
    };

private:
    Managed<LensData> _lens_data{1};

public:
    explicit ThinLensCamera(const SensorDesc &desc)
        : Camera(desc) {
        _lens_data.emplace_back();
        _lens_data->focal_distance = desc["focal_distance"].as_float(5.f);
        _lens_data->lens_radius = desc["lens_radius"].as_float(0.f);
    }
    void update_device_data() noexcept override {
        Camera::update_device_data();
        _lens_data.upload_immediately();
    }
    void prepare() noexcept override {
        Camera::prepare();
        _lens_data.reset_device_buffer_immediately(pipeline()->device(), 1);
    }
};
}// namespace vision

OC_STRUCT(vision::ThinLensCamera::LensData, focal_distance, lens_radius){};

VS_MAKE_CLASS_CREATOR(vision::ThinLensCamera)