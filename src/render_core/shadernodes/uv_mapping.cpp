//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class UVMapping : public ShaderNode {
private:
    float2 _offset{};
    float2 _scale{};

public:
    explicit UVMapping(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};
}// namespace vision