//
// Created by Zero on 2023/6/19.
//

#include "surface.h"

namespace vision {

Surface::Surface(const vision::SensorDesc &desc)
    : Sensor(desc) {}

void Surface::update_data(RegistrableManaged<ocarina::float4> &positions,
                          RegistrableManaged<ocarina::float4> &normals) noexcept {
    _positions = &positions;
    _normals = &normals;
}

RayState Surface::generate_ray(const vision::SensorSample &ss) const noexcept {
    return {};
}

}// namespace vision