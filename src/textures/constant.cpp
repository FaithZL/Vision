//
// Created by Zero on 09/09/2022.
//

#include "base/texture.h"

namespace vision {
class ConstantTexture : public Texture {
private:
    float4 _val;

public:
    explicit ConstantTexture(const TextureDesc *desc) : Texture(desc) {}
    [[nodiscard]] Float4 eval(const Float2 &uv) const noexcept override { return _val; }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ConstantTexture)