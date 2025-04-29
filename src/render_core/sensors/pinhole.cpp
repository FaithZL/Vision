//
// Created by Zero on 2023/7/18.
//

#include "base/sensor/sensor.h"

namespace vision {

class PinholeCamera : public Sensor {
public:
    explicit PinholeCamera(const SensorDesc &desc)
        : Sensor(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PinholeCamera)