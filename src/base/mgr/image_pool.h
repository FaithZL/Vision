//
// Created by Zero on 26/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "descriptions/node_desc.h"
#include "util/image_io.h"

namespace vision {
using namespace ocarina;

class RenderPipeline;

class ImageWrapper {
private:
    ImageIO _image_io;
    RHITexture _texture;
    uint _id;

public:
    ImageWrapper() = default;
    ImageWrapper(ImageIO image_io, RHITexture image, uint id)
        : _image_io(move(image_io)), _texture(move(image)), _id(id) {}
    [[nodiscard]] RHITexture &texture() noexcept { return _texture; }
    [[nodiscard]] const RHITexture &texture() const noexcept { return _texture; }
    [[nodiscard]] const ImageIO &image() const noexcept { return _image_io; }
    [[nodiscard]] ImageIO &image() noexcept { return _image_io; }
    [[nodiscard]] uint id() const noexcept { return _id; }
    [[nodiscard]] static ImageWrapper create(const ShaderNodeDesc &desc, RenderPipeline *rp);
    [[nodiscard]] TextureUploadCommand *upload() const noexcept;
    [[nodiscard]] TextureDownloadCommand *download() noexcept;
    void upload_immediately() const noexcept;
    void download_immediately() noexcept;
};

class ImagePool {
private:
    RenderPipeline *_rp{nullptr};
    map<uint64_t, ImageWrapper> _images;

public:
    explicit ImagePool(RenderPipeline *rp) : _rp(rp) {}
    [[nodiscard]] ImageWrapper &obtain_image(const ShaderNodeDesc &desc) noexcept;
    void prepare() noexcept;
    [[nodiscard]] bool is_contain(uint64_t hash) const noexcept { return _images.contains(hash); }
};

}// namespace vision