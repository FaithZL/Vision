//
// Created by Zero on 2023/6/19.
//

#pragma once

#include "base/sensor/sensor.h"

namespace vision {

class Surface : public Sensor {
private:
    RegistrableManaged<float4> *_positions{};
    RegistrableManaged<float4> *_normals{};

public:
    explicit Surface(const SensorDesc &desc);
    void update_data(RegistrableManaged<float4>&positions, RegistrableManaged<float4> &normals) noexcept;
    [[nodiscard]] RayState generate_ray(const vision::SensorSample &ss) const noexcept override;
};

}// namespace vision