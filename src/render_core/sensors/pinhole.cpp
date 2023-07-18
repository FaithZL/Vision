//
// Created by Zero on 2023/7/18.
//

#include "base/sensor/camera.h"

namespace vision {

class PinholeCamera : public Camera {
public:
    explicit PinholeCamera(const SensorDesc &desc)
        : Camera(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PinholeCamera)