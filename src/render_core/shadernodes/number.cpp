//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"
#include "GUI/widgets.h"

namespace vision {
class NumberInput : public ShaderNode {
private:
    Serial<vector<float>> _value;

public:
    explicit NumberInput(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _value(desc["value"].as_vector<float>()) {}
    OC_SERIALIZABLE_FUNC(ShaderNode, _value)
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        _changed = widgets->color_edit("pure color", reinterpret_cast<float3 *>(_value.hv().data()));
        return true;
    }
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] bool is_zero() const noexcept override { return false; }
    [[nodiscard]] bool is_constant() const noexcept override { return false; }
    [[nodiscard]] uint dim() const noexcept override { return _value.element_num(); }
    [[nodiscard]] bool is_uniform() const noexcept override { return true; }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return _value.hv();
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64_list(_value.hv());
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return *_value;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NumberInput)