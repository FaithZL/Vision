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
        return t_->is_uniform() && A_->is_uniform() && B_->is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return t_->is_constant() && A_->is_constant() && B_->is_constant();
    }
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(t_.type_hash(), A_.type_hash(), B_.type_hash());
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(t_.hash(), A_.hash(), B_.hash());
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return ocarina::lerp(t_.average(), A_.average(), B_.average());
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return ocarina::lerp(t_.evaluate(ctx, swl), A_.evaluate(ctx, swl), B_.evaluate(ctx, swl));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Lerp)