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
    [[nodiscard]] AttrEvalContext evaluate(const string &key, const AttrEvalContext &ctx,
                                          const SampledWavelengths &swl) const noexcept override {
        Float3 uvw;
        if (key == "Generated") {
            uvw = ctx.uvw();
        } else if (key == "Normal") {
            uvw = ctx.ng();
        } else if (key == "UV") {
            uvw = ctx.uvw();
        } else if (key == "Camera") {
            TSensor sensor = scene().sensor();
            Float3 c_pos = transform_point(sensor->device_c2w(), ctx.pos());
            uvw = c_pos;
        } else if (key == "Object") {
            uvw = ctx.pos();
        } else if (key == "Window") {
            uvw = make_float3(make_float2(dispatch_idx().xy()) / dispatch_dim().xy(), 0.f);
        } else {
            OC_WARNING_FORMAT("{} is unknown, fallback to uv", key.c_str());
            uvw = ctx.uvw();
        }
        return float_array::from_vec(uvw);
    }
};
//todo
class GeometryNode : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(geometry)

    [[nodiscard]] AttrEvalContext evaluate(const string &key, const AttrEvalContext &ctx,
                                          const SampledWavelengths &swl) const noexcept override {
        Float3 uvw;
        if (key == "Position") {
            uvw = ctx.pos();
        } else if (key == "Normal") {
            uvw = ctx.ns();
        } else if (key == "Tangent") {
            uvw = ctx.tangent();
        } else if (key == "True Normal") {
            uvw = ctx.ng();
        } else if (key == "Incoming") {
            uvw = ctx.wo();
        } else {
            OC_WARNING_FORMAT("{} is unknown, fallback to uv", key.c_str());
            uvw = ctx.uvw();
        }
        return float_array::from_vec(uvw);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, TextureCoordinate, tex_coord)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, GeometryNode, geometry)
VS_REGISTER_CURRENT_FILE