//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"
#include "GUI/widgets.h"

namespace vision {
class NumberInput : public ShaderNode {
private:
    EncodedData<vector<float>> value_;
    EncodedData<float> intensity_{1.f};
    bool sync_{true};

public:
    explicit NumberInput(const ShaderNodeDesc &desc)
        : ShaderNode(desc), value_(desc["value"].as_vector<float>()) {
        if (type_ == Illumination) {
            float max_v = *std::max_element(value_.hv().begin(), value_.hv().end());
            intensity_ = max_v;
            std::transform(value_.hv().begin(), value_.hv().end(), value_.hv().begin(), [&](auto v) {
                return v / max_v;
            });
        }
    }
    OC_ENCODABLE_FUNC(ShaderNode, value_, intensity_)
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        auto &values = value_.hv();
        switch (type_) {
            case ShaderNodeType::Number: {
                if (values.size() > 1) {
                    widgets->check_box(ocarina::format("{} sync", name_.c_str()), &sync_);
                }
                if (sync_) {
                    changed_ |= widgets->drag_floatN(name_, values.data(), 1, 0.01);
                    for (int i = 1; i < values.size(); ++i) {
                        values[i] = values[0];
                    }
                } else {
                    changed_ |= widgets->drag_floatN(name_, values.data(), values.size(),0.01);
                }
                break;
            }
            case ShaderNodeType::Albedo: {
                changed_ |= widgets->colorN_edit(name_, values.data(), values.size());
                break;
            }
            case ShaderNodeType::Illumination: {
                changed_ |= widgets->colorN_edit(ocarina::format("intensity:{} {}", intensity_.hv(), name_.c_str()),
                                                 values.data(), values.size());
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
    [[nodiscard]] uint dim() const noexcept override { return value_.element_num(); }
    [[nodiscard]] bool is_uniform() const noexcept override { return true; }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return value_.hv();
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64_list(value_.hv());
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept override {
        return *value_ * *intensity_;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NumberInput)