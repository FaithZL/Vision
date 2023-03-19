//
// Created by Zero on 09/09/2022.
//

#include "base/shader_graph/shader_node.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
class Constant : public ShaderNode {
private:
    float4 _val;

public:
    explicit Constant(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _val(desc["value"].as_float4()) {}
    [[nodiscard]] bool is_zero() const noexcept override { return ocarina::is_zero(_val); }
    [[nodiscard]] bool is_constant() const noexcept override { return true; }
    [[nodiscard]] bool is_versatile() const noexcept override { return false; }
    [[nodiscard]] Float4 eval(const AttrEvalContext &tev) const noexcept override { return _val; }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Constant)