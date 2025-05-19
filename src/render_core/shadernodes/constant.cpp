//
// Created by Zero on 09/09/2022.
//

#include "base/shader_graph/shader_node.h"
#include "base/mgr/pipeline.h"

namespace vision {
class Constant : public ShaderNode {
private:
    vector<float> value_;

public:
    Constant() = default;
    explicit Constant(const ShaderNodeDesc &desc)
        : ShaderNode(desc), value_(desc["value"].as_vector<float>()) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, value_)
    [[nodiscard]] bool near_zero() const noexcept override {
        return std::all_of(value_.begin(), value_.end(), [](float elm) { return ocarina::abs(elm - 0.f) < s_cutoff; });
    }
    [[nodiscard]] bool near_one() const noexcept override {
        return std::all_of(value_.begin(), value_.end(), [](float elm) { return ocarina::abs(elm - 1.f) < s_cutoff; });
    }
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return hash64_list(value_);
    }
    [[nodiscard]] uint64_t compute_hash() const noexcept override {
        return hash64_list(value_);
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return value_;
    }
    [[nodiscard]] bool is_constant() const noexcept override { return true; }
    [[nodiscard]] uint dim() const noexcept override { return value_.size(); }
    [[nodiscard]] bool is_uniform() const noexcept override { return true; }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return DynamicArray<float>(value_);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, Constant)