//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"

#include "base/shape.h"

namespace vision {

class Cube : public vision::Mesh {
public:
    using Super = vision::Mesh;

public:
    explicit Cube(const ShapeDesc *desc) : Super(desc) {}
    void init(const ShapeDesc *desc) noexcept {
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Cube)