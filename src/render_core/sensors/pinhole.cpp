//
// Created by Zero on 2023/7/18.
//

#include "base/sensor/camera.h"

namespace vision {

class PinholeCamera : public CameraImpl {
public:
    explicit PinholeCamera(const SensorDesc &desc)
        : CameraImpl(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PinholeCamera)