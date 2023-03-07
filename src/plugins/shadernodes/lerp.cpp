//
// Created by Zero on 07/03/2023.
//

#include "base/shader_node.h"

namespace vision {
class Lerp : public ShaderNode {
private:
    const ShaderNode *_t{};
    const ShaderNode *A{};
    const ShaderNode *B{};

public:
    explicit Lerp(const ShaderNodeDesc &desc) : ShaderNode(desc) {}

    [[nodiscard]] Float4 eval(const AttrEvalContext &tec) const noexcept override {
        return lerp(_t->eval(tec), A->eval(tec), B->eval(tec));
    }

    [[nodiscard]] Float4 eval(const Float2 &uv) const noexcept override {
        return lerp(_t->eval(uv), A->eval(uv), B->eval(uv));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Lerp)