//
// Created by Zero on 09/09/2022.
//

#include "base/texture.h"
#include "rhi/common.h"

namespace vision {
using namespace ocarina;
class ImageTexture : public Texture {
private:
    Image _image;
    float4 _val;

public:
    explicit ImageTexture(const TextureDesc &desc)
        : Texture(desc) {}
    [[nodiscard]] Float4 eval(const TextureEvalContext &tev) const noexcept override { return _val; }
    [[nodiscard]] Float4 eval(const Float2 &uv) const noexcept override { return _val; }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ImageTexture)