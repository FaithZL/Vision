//
// Created by Zero on 09/09/2022.
//

#include "base/shader_node.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
class Constant : public ShaderNode {
private:
    float4 _val;

public:
    explicit Constant(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _val(desc["value"].as_float4()) {}
    void prepare() noexcept override {
        switch (_type) {
            case ShaderNodeType::Albedo:
                _val = spectrum().albedo_params(_val);
                break;
            case ShaderNodeType::Illumination:
                _val = spectrum().illumination_params(_val);
                break;
            case ShaderNodeType::Unbound:
                _val = spectrum().unbound_params(_val);
                break;
            default:
                break;
        }
    }
    [[nodiscard]] bool is_zero() const noexcept override { return ocarina::is_zero(_val); }
    [[nodiscard]] Float4 eval(const AttrEvalContext &tev) const noexcept override { return _val; }
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const Float2 &uv,
                                                   const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_albedo(_val, swl);
    }
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const Float2 &uv,
                                                         const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_illumination(_val, swl);
    }
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const AttrEvalContext &tec,
                                                   const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_albedo(_val, swl);
    }
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const AttrEvalContext &tec,
                                                         const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_illumination(_val, swl);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Constant)