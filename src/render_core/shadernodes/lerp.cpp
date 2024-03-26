//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class Lerp : public ShaderNode {
private:
    VS_MAKE_SLOT(t)
    VS_MAKE_SLOT(A)
    VS_MAKE_SLOT(B)

public:
    explicit Lerp(const ShaderNodeDesc &desc) : ShaderNode(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] bool is_uniform() const noexcept override {
        return _t->is_uniform() && _A->is_uniform() && _B->is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return _t->is_constant() && _A->is_constant() && _B->is_constant();
    }
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_t.type_hash(), _A.type_hash(), _B.type_hash());
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_t.hash(), _A.hash(), _B.hash());
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return ocarina::lerp(_t.average(), _A.average(), _B.average());
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return ocarina::lerp(_t.evaluate(ctx, swl),_A.evaluate(ctx, swl),_B.evaluate(ctx, swl));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Lerp)