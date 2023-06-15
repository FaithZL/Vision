//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "node.h"

namespace vision {
using namespace ocarina;

class Rasterizer : public Node {
public:
    using Desc = RasterizerDesc;

public:
    explicit Rasterizer(const RasterizerDesc &desc)
        : Node(desc) {}
};

}// namespace vision