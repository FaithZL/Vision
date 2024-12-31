//
// Created by Zero on 2025/1/1.
//

#pragma once

#include "node.h"

namespace vision {

class Coloured : public Node {
public:
    using Node::Node;
    [[nodiscard]] virtual bool has_emission() const noexcept { return false; }
};

}// namespace vision