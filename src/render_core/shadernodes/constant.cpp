//
// Created by Zero on 09/09/2022.
//

#include "base/shader_graph/shader_node.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
class Constant : public ShaderNode {
private:
    vector<float> _value;

public:
    explicit Constant(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _value(desc["value"].as_vector<float>()) {}
    [[nodiscard]] bool is_zero() const noexcept override { return false; }
    [[nodiscard]] bool is_constant() const noexcept override { return true; }
    [[nodiscard]] bool is_uniform() const noexcept override { return true; }
    [[nodiscard]] Float4 eval(const AttrEvalContext &tev) const noexcept override { return Array<float>(_value).to_vec4(); }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Constant)