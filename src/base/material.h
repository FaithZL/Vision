//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"

namespace vision {
class Material : public Node {
public:
    using Desc = MaterialDesc;

public:
    explicit Material(const MaterialDesc &desc) : Node(desc) {}
};
}// namespace vision