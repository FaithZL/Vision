//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class Lerp : public ShaderNode {
private:
    const ShaderNode *_t{};
    const ShaderNode *A{};
    const ShaderNode *B{};

public:
    explicit Lerp(const ShaderNodeDesc &desc) : ShaderNode(desc) {}
    [[nodiscard]] bool is_uniform() const noexcept override {
        return _t->is_uniform() && A->is_uniform() && B->is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return _t->is_constant() && A->is_constant() && B->is_constant();
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx) const noexcept override {
        return Array<float>{1u};
    }
    [[nodiscard]] Float4 eval(const AttrEvalContext &tec) const noexcept override {
        return lerp(_t->eval(tec), A->eval(tec), B->eval(tec));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Lerp)