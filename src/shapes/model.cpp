//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"

namespace vision {

class Model : public Shape {
public:
    explicit Model(const ShapeDesc &desc) : Shape(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)