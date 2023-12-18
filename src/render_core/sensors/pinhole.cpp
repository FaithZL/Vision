//
// Created by Zero on 2023/7/18.
//

#include "base/sensor/camera.h"

namespace vision {

class PinholeCamera : public Camera {
public:
    explicit PinholeCamera(const SensorDesc &desc)
        : Camera(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PinholeCamera)