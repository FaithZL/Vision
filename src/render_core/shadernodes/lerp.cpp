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
        return _t->is_uniform() && A->is_uniform() && B->is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return _t->is_constant() && A->is_constant() && B->is_constant();
    }
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_t.type_hash(), A.type_hash(), B.type_hash());
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_t.hash(), A.hash(), B.hash());
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return ocarina::lerp(_t.average(), A.average(), B.average());
    }
//    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
//                                        const SampledWavelengths &swl) const noexcept override {
//        return ocarina::lerp(_t.evaluate(ctx, swl),A.evaluate(ctx, swl),B.evaluate(ctx, swl));
//    }
};
}// namespace vision

//VS_MAKE_CLASS_CREATOR(vision::Lerp)