//
// Created by ling.zhu on 2025/5/2.
//

#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

class TextureCoordinate : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(tex_coord)
    [[nodiscard]] AttrEvalOutput evaluate(const string &key, const AttrEvalInput &ctx,
                                          const SampledWavelengths &swl) const noexcept override {
        Float2 uv;
        if (key == "Generated") {
            uv = ctx.uv;
        } else if (key == "Normal") {
            // todo
            uv = ctx.uv;
        } else if (key == "UV") {
            uv = ctx.uv;
        } else if (key == "Camera") {
            TSensor sensor = scene().sensor();
            uv = sensor->device_position().xy();
        } else if (key == "Object") {
            uv = ctx.pos->xy();
        } else if (key == "Window") {
            uv = make_float2(dispatch_idx().xy()) / dispatch_dim().xy();
        } else {
            OC_WARNING_FORMAT("{} is unknown, fallback to uv", key.c_str());
            uv = ctx.uv;
        }
        return float_array::from_vec(uv);
    }
};
//todo
class GeometryNode : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(geometry)

    [[nodiscard]] AttrEvalOutput evaluate(const string &key, const AttrEvalInput &ctx,
                                          const SampledWavelengths &swl) const noexcept override {
        Float2 uv;
        if (key == "Position") {
            uv = ctx.uv;
        } else if (key == "Normal") {
            TSensor sensor = scene().sensor();
            uv = sensor->device_position().xy();
        } else if (key == "Tangent") {
            uv = ctx.pos->xy();
        } else if (key == "True Normal") {
            uv = make_float2(dispatch_idx().xy()) / dispatch_dim().xy();
        } else if (key == "Incoming") {
            uv = make_float2(dispatch_idx().xy()) / dispatch_dim().xy();
        } else {
            OC_WARNING_FORMAT("{} is unknown, fallback to uv", key.c_str());
            uv = ctx.uv;
        }
        return float_array::from_vec(uv);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, TextureCoordinate, tex_coord)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, GeometryNode, geometry)
VS_REGISTER_CURRENT_FILE