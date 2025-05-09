//
// Created by ling.zhu on 2025/5/2.
//

#include "base/shader_graph/shader_node.h"

namespace vision {

class TextureCoordinate : public SourceNode {
public:
    using SourceNode::SourceNode;
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] AttrEvalContext apply(const AttrEvalContext &ctx, const SampledWavelengths &swl,
                                        const string &key) const noexcept override {

        return ctx;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, TextureCoordinate)
VS_REGISTER_CURRENT_PATH(0, "vision-shadernode-tex_coord.dll")