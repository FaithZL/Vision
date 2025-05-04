//
// Created by ling.zhu on 2025/5/2.
//

#include "base/shader_graph/shader_node.h"

namespace vision {

class TextureCoordinate : public InputNode {
public:
    using InputNode::InputNode;
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] AttrEvalContext apply(const AttrEvalContext &ctx,
                                        const string &key) const noexcept override {

        return ctx;
    }
};

}

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, TextureCoordinate)
VS_REGISTER_CURRENT_PATH(0, "vision-shadernode-tex_coord.dll")