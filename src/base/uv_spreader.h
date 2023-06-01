//
// Created by Zero on 2023/6/2.
//

#pragma once

#include "core/basic_types.h"
#include "node.h"

namespace vision {

using namespace ocarina;

class UVSpreader : public Node {
public:
    using Desc = UVSpreaderDesc;

public:
    explicit UVSpreader(const UVSpreaderDesc &desc)
        : Node(desc) {}
};

}// namespace vision