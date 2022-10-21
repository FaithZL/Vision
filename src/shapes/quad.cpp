//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"

namespace vision {

class Quad : public vision::Mesh {
public:
    using Super = vision::Mesh;

public:
    explicit Quad(const ShapeDesc *desc) : Super(desc) {}
    void init(const ShapeDesc *desc) noexcept {
        float width = desc->width / 2;
        float height = desc->height / 2;
        vector<float3> P{make_float3(width, 0, height),
                         make_float3(width, 0, -height),
                         make_float3(-width, 0, height),
                         make_float3(-width, 0, -height)};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Quad)