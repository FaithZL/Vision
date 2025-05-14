//
// Created by ling.zhu on 2025/5/2.
//

#include "base/shader_graph/shader_node.h"

namespace vision {

class TextureCoordinate : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC

    [[nodiscard]] float_array evaluate(const string &key, const AttrEvalContext &ctx,
                                       const SampledWavelengths &swl) const noexcept override {
        if (key == "UV") {
            return float_array::create(ctx.uv);
        }
        return {};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, TextureCoordinate)
VS_REGISTER_CURRENT_PATH(0, "vision-shadernode-tex_coord.dll")