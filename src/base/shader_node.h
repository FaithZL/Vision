//
// Created by Zero on 06/03/2023.
//

#pragma once

#include "node.h"
#include "descriptions/node_desc.h"

namespace vision {

class ShaderNode : public Node {
public:
    using Desc = ShaderNodeDesc;

public:
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc) {}

    
};

}// namespace vision