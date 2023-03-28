//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class NumberInput : public ShaderNode {
private:
    vector<float> _value;

public:
    explicit NumberInput(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _value(desc["value"].as_vector<float>()) {}
    [[nodiscard]] bool is_zero() const noexcept override { return false; }
    [[nodiscard]] bool is_constant() const noexcept override { return false; }
    [[nodiscard]] uint dim() const noexcept override { return _value.size(); }
    [[nodiscard]] bool is_uniform() const noexcept override { return true; }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx) const noexcept override {
        return Array<float>(_value);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NumberInput)