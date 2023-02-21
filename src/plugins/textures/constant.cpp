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
    explicit ConstantTexture(const TextureDesc &desc) : Texture(desc), _val(desc.val) {}
    void prepare() noexcept override {
        switch (_type) {
            case SpectrumType::Albedo:
                _val = spectrum().albedo_params(_val);
                break;
            case SpectrumType::Illumination:
                _val = spectrum().illumination_params(_val);
                break;
            default:
                break;
        }
    }
    [[nodiscard]] bool is_zero() const noexcept override { return ocarina::is_zero(_val); }
    [[nodiscard]] Float4 eval(const TextureEvalContext &tev) const noexcept override { return _val; }
    [[nodiscard]] Float4 eval(const Float2 &uv) const noexcept override { return _val; }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ConstantTexture)