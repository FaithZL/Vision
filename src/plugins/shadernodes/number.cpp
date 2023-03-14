//
// Created by Zero on 07/03/2023.
//

#include "base/shader_node.h"

namespace vision {
class NumberInput : public ShaderNode {
private:
    float4 _val;

public:
    explicit NumberInput(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _val(desc["value"].as_float4()) {}
};
}// namespace vision