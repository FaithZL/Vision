//
// Created by Zero on 2023/6/19.
//

#include "surface.h"

namespace vision {

Surface::Surface(const vision::SensorDesc &desc)
    : Sensor(desc) {}

RayState Surface::generate_ray(const vision::SensorSample &ss) const noexcept {
    return {};
}

}// namespace vision