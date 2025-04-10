//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"
#include "GUI/widgets.h"

namespace vision {
class NumberInput : public ShaderNode {
private:
    EncodedData<vector<float>> value_;
    float min_{0.f};
    float max_{1.f};
    bool sync_{false};

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return hash64_list(value_.hv()); }
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override { return hash64(value_.hv().size(), min_, max_); }

public:
    NumberInput() = default;
    explicit NumberInput(const ShaderNodeDesc &desc)
        : ShaderNode(desc),
          value_(desc["value"].as_vector<float>()),
          min_(desc["min"].as_float(0.f)),
          max_(desc["max"].as_float(1.f)) {
        update_encode_type();
    }
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, value_, sync_, min_, max_)
    OC_ENCODABLE_FUNC(ShaderNode, value_)

    void update_encode_type() noexcept {
//        if ((min_ >= 0.f && max_ <= 1.f) || node_tag() == Albedo || node_tag() == Illumination) {
//            value_.set_encode_type(Uint8);
//        } else {
//            value_.set_encode_type(Original);
//        }
    }

    [[nodiscard]] float normalize() noexcept override {
        float max_v = *std::max_element(value_.hv().begin(), value_.hv().end());
        if (max_v < 1.f) {
            return 1.f;
        }
        std::transform(value_.hv().begin(), value_.hv().end(), value_.hv().begin(), [&](auto v) {
            return v / max_v;
        });
        return max_v;
    }

    ShaderNode &set_range(float lower, float upper) noexcept override {
        min_ = lower;
        max_ = upper;
        update_encode_type();
        return *this;
    }

    ShaderNode &update_value(vector<float> values) noexcept override {
        value_.hv() = std::move(values);
        return *this;
    }

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        auto &values = value_.hv();
        switch (node_tag_) {
            case SlotTag::Number: {
                if (values.size() > 1) {
                    widgets->check_box(ocarina::format("{} sync", name_.c_str()), &sync_);
                }
                if (sync_) {
                    changed_ |= widgets->drag_floatN(name_, values.data(), 1, 0.01, min_, max_);
                    for (int i = 1; i < values.size(); ++i) {
                        values[i] = values[0];
                    }
                } else {
                    changed_ |= widgets->drag_floatN(name_, values.data(), values.size(), 0.01, min_, max_);
                }
                break;
            }
            case SlotTag::Albedo:
            case SlotTag::Illumination: {
                changed_ |= widgets->colorN_edit(name_, values.data(), values.size());
                break;
            }
            default:
                break;
        }
        return true;
    }
    
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] bool near_zero() const noexcept override {
        auto lst = value_.hv();
        return std::all_of(lst.begin(), lst.end(), [](float elm) { return ocarina::abs(elm - 0.f) < s_cutoff; });
    }
    [[nodiscard]] bool near_one() const noexcept override {
        auto lst = value_.hv();
        return std::all_of(lst.begin(), lst.end(), [](float elm) { return ocarina::abs(elm - 1.f) < s_cutoff; });
    }
    [[nodiscard]] bool is_constant() const noexcept override { return false; }
    [[nodiscard]] uint dim() const noexcept override { return value_.hv().size(); }
    [[nodiscard]] bool is_uniform() const noexcept override { return true; }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return value_.hv();
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept override {
        return *value_;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, NumberInput)
VS_REGISTER_CURRENT_PATH(0, "vision-shadernode-number.dll")