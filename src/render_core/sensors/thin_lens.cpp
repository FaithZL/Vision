//
// Created by Zero on 09/09/2022.
//

#include "base/sensor/sensor.h"
#include "base/mgr/pipeline.h"

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
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ThinLensCamera)