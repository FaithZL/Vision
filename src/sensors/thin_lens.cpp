//
// Created by Zero on 09/09/2022.
//

#include "base/sensor.h"

namespace vision {
struct ThinLensCameraData : public CameraData {
    float focal_distance{};
    float lens_radius{0.f};
};
}// namespace vision

OC_STRUCT(vision::ThinLensCameraData, position,fov_y,yaw,pitch,
          velocity,sensitivity,raster_to_screen,camera_to_screen,
          raster_to_camera,focal_distance,lens_radius) {};

namespace vision {
class ThinLensCamera : public Camera {
private:
    ThinLensCameraData _data;

public:
    explicit ThinLensCamera(SensorDesc *desc)
        : Camera(desc, &_data) {

    }
};
}// namespace vision

//VS_MAKE_CLASS_CREATOR(vision)