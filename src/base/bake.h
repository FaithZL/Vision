//
// Created by Zero on 2023/6/2.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"
#include "shape.h"

namespace vision {

using namespace ocarina;

struct BakedShape {
    Shape *shape{};
    uint2 resolution{};
    RegistrableManaged<float4> normal;
    RegistrableManaged<float4> position;
};


class UVSpreader : public Node {
public:
    using Desc = UVSpreaderDesc;

public:
    explicit UVSpreader(const UVSpreaderDesc &desc)
        : Node(desc) {}
    [[nodiscard]] virtual BakedShape apply(vision::Shape *shape) = 0;
};

class Rasterizer : public Node {
public:
    using Desc = RasterizerDesc;

public:
    explicit Rasterizer(const RasterizerDesc &desc)
        : Node(desc) {}

    virtual void apply(BakedShape &baked_shape) noexcept = 0;
};

}// namespace vision