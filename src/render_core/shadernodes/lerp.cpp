//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class Lerp : public ShaderNode {
private:
    Slot _t{};
    Slot A{};
    Slot B{};

public:
    explicit Lerp(const ShaderNodeDesc &desc) : ShaderNode(desc) {}
    [[nodiscard]] bool is_uniform() const noexcept override {
        return _t.is_uniform() && A.is_uniform() && B.is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return _t.is_constant() && A.is_constant() && B.is_constant();
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx) const noexcept override {
        return Array<float>{1u};
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Lerp)