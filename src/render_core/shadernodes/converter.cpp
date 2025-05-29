//
// Created by ling.zhu on 2025/5/22.
//

#include "base/shader_graph/shader_node.h"
#include "GUI/widgets.h"

namespace vision {

class VectorMapping : public ShaderNode {
public:
    using ShaderNode::ShaderNode;
    VS_MAKE_PLUGIN_NAME_FUNC_(vector_mapping)

private:
    VS_MAKE_SLOT(vector);
    EncodedData<float3> location_;
    EncodedData<float3> rotation_;
    EncodedData<float3> scale_;

public:
    VectorMapping() = default;
    explicit VectorMapping(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          location_(desc["location"].as_float3()),
          rotation_(desc["rotation"].as_float3()),
          scale_(desc["scale"].as_float3()) {}

    [[nodiscard]] AttrEvalContext evaluate(const std::string &key, const vision::AttrEvalContext &ctx,
                                           const vision::SampledWavelengths &swl) const noexcept override {

        return ctx;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX_FUNC(vision, VectorMapping, vector_mapping)
VS_REGISTER_CURRENT_FILE