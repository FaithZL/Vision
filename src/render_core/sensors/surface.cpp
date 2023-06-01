//
// Created by Zero on 2023/6/2.
//

#include "base/sensor.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

class Surface : public Sensor {
private:
    Slot _position;
    Slot _normal;

public:
    explicit Surface(const SensorDesc &desc) : Sensor(desc) {}
};

}// namespace vision