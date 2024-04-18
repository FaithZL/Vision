//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class UVMapping : public ShaderNode {
private:
    Serial<float2> offset_{};
    Serial<float2> scale_{};

public:
    explicit UVMapping(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};
}// namespace vision