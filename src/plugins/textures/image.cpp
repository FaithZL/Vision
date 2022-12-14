//
// Created by Zero on 09/09/2022.
//

#include "base/texture.h"
#include "rhi/common.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
using namespace ocarina;
class ImageTexture : public Texture {
private:
    const ImageWrapper &_image_wrapper;

public:
    explicit ImageTexture(const TextureDesc &desc)
        : Texture(desc),
          _image_wrapper(desc.scene->render_pipeline()->obtain_image(desc)) {
        _image_wrapper.upload_immediately();
    }
    [[nodiscard]] bool is_zero() const noexcept override { return false; }
    [[nodiscard]] Float4 eval(const TextureEvalContext &tev) const noexcept override {
        return eval(tev.uv);
    }
    [[nodiscard]] Float4 eval(const Float2 &uv) const noexcept override {
        return _image_wrapper.image().sample<float4>(uv);
    }
    [[nodiscard]] uint2 resolution() const noexcept override {
        return _image_wrapper.image().resolution();
    }
    void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept override {
        _image_wrapper.host_image().for_each_pixel(func);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ImageTexture)