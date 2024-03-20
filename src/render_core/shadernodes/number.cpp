//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"
#include "GUI/widgets.h"

namespace vision {
class NumberInput : public ShaderNode {
private:
    Serial<vector<float>> _value;
    Serial<float> _intensity{1.f};

public:
    explicit NumberInput(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _value(desc["value"].as_vector<float>()) {
        if (_type == Illumination) {
            float max_v = *std::max_element(_value.hv().begin(), _value.hv().end());
            _intensity = max_v;
            std::transform(_value.hv().begin(), _value.hv().end(), _value.hv().begin(), [&](auto v) {
                return v / max_v;
            });
        }
    }
    OC_SERIALIZABLE_FUNC(ShaderNode, _value, _intensity)
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        auto &values = _value.hv();
        switch (_type) {
            case ShaderNodeType::Number: {
                _changed |= widgets->input_floatN("", values.data(), values.size());
                break;
            }
            case ShaderNodeType::Albedo: {
                _changed |= widgets->colorN_edit("", values.data(), values.size());
                break;
            }
            case ShaderNodeType::Illumination: {
                _changed |= widgets->colorN_edit(ocarina::format("intensity:{}", _intensity.hv()), values.data(), values.size());
                break;
            }
            default:
                break;
        }
        return true;
    }
    VS_MAKE_PLUGIN_NAME_FUNC
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
        return *_value * *_intensity;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NumberInput)