//
// Created by zhu on 2023/4/18.
//

#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {
class Multiply : public ShaderNode {
private:
    Slot _lhs;
    Slot _rhs;

public:
    explicit Multiply(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          _lhs(desc.scene->create_slot(*desc.slot("lhs", desc["lhs"].data(), Albedo))),
          _rhs(desc.scene->create_slot(*desc.slot("rhs", desc["rhs"].data(), Albedo))) {}

    [[nodiscard]] bool is_uniform() const noexcept override {
        return _lhs->is_uniform() && _rhs->is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return _lhs->is_constant() && _rhs->is_constant();
    }

    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return _lhs.average() * _rhs.average();
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return _lhs.evaluate(ctx, swl) * _rhs.evaluate(ctx, swl);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Multiply)