//
// Created by Zero on 09/09/2022.
//

#include "base/texture.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
class ConstantTexture : public Texture {
private:
    float4 _val;

public:
    explicit ConstantTexture(const ShaderNodeDesc &desc) : Texture(desc), _val(desc.val) {}
    void prepare() noexcept override {
        switch (_type) {
            case AttrType::Albedo:
                _val = spectrum().albedo_params(_val);
                break;
            case AttrType::Illumination:
                _val = spectrum().illumination_params(_val);
                break;
            case AttrType::Unbound:
                _val = spectrum().unbound_params(_val);
                break;
            default:
                break;
        }
    }
    [[nodiscard]] bool is_zero() const noexcept override { return ocarina::is_zero(_val); }
    [[nodiscard]] Float4 eval(const TextureEvalContext &tev) const noexcept override { return _val; }
    [[nodiscard]] Float4 eval(const Float2 &uv) const noexcept override { return _val; }
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const Float2 &uv,
                                                   const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_albedo(_val, swl);
    }
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const Float2 &uv,
                                                         const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_illumination(_val, swl);
    }
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const TextureEvalContext &tec,
                                                   const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_albedo(_val, swl);
    }
    [[nodiscard]] ColorDecode eval_illumination_spectrum(const TextureEvalContext &tec,
                                                         const SampledWavelengths &swl) const noexcept override {
        return spectrum().params_to_illumination(_val, swl);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ConstantTexture)