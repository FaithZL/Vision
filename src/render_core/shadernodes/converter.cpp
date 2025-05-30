//
// Created by ling.zhu on 2025/5/22.
//

#include "base/shader_graph/shader_graph.h"
#include "GUI/widgets.h"

namespace vision {

class FresnelNode : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(fresnel)

    [[nodiscard]] AttrEvalContext evaluate(const std::string &key, const vision::AttrEvalContext &ctx,
                                           const vision::SampledWavelengths &swl) const noexcept override {

        return ctx;
    }
};

class VectorMapping : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(vector_mapping)

private:
    std::string type_;
    VS_MAKE_SLOT(vector);
    VS_MAKE_SLOT(location);
    VS_MAKE_SLOT(rotation);
    VS_MAKE_SLOT(scale);

public:
    VectorMapping() = default;
    explicit VectorMapping(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}

    VS_MAKE_GUI_ALL_FUNC(ShaderNode, vector_, location_, rotation_, scale_)
    OC_ENCODABLE_FUNC(ShaderNode, vector_, location_, rotation_, scale_)

    void initialize_slots(const vision::ShaderNodeDesc &desc) noexcept override {
        VS_INIT_SLOT_NO_DEFAULT(vector, Number);
        VS_INIT_SLOT_NO_DEFAULT(location, Number);
        VS_INIT_SLOT_NO_DEFAULT(rotation, Number);
        VS_INIT_SLOT_NO_DEFAULT(scale, Number);
    }

    [[nodiscard]] AttrEvalContext evaluate(const std::string &key, const vision::AttrEvalContext &ctx,
                                           const vision::SampledWavelengths &swl) const noexcept override {

        return ctx;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, FresnelNode, fresnel)
VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, VectorMapping, vector_mapping)
VS_REGISTER_CURRENT_FILE