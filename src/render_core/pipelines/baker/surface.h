//
// Created by Zero on 2023/6/19.
//

#pragma once

#include "base/sensor/sensor.h"

namespace vision {

class Surface : public Sensor {
private:
    uint _position_index{InvalidUI32};
    uint _normal_index{InvalidUI32};

public:
    explicit Surface(const SensorDesc &desc);
    void init(uint p_index, uint n_index) {
        _position_index = p_index;
        _normal_index = n_index;
    }
    [[nodiscard]] RayState generate_ray(const vision::SensorSample &ss) const noexcept override;
};

}// namespace vision