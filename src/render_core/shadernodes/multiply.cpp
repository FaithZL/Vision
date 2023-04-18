//
// Created by zhu on 2023/4/18.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class Multiply : public ShaderNode {
private:
    Slot _lhs;
    Slot _rhs;

public:
    explicit Multiply(const ShaderNodeDesc &desc) : ShaderNode(desc) {}
};
}// namespace vision