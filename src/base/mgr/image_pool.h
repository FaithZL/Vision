//
// Created by Zero on 26/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "descriptions/node_desc.h"
#include "util/image_io.h"

namespace vision {
using namespace ocarina;

class ImageWrapper {
private:
    ImageIO _image_io;
    RHITexture _texture;

public:
    ImageWrapper() = default;
    ImageWrapper(ImageIO image_io, RHITexture image) : _image_io(move(image_io)), _texture(move(image)) {}
    [[nodiscard]] RHITexture &texture() noexcept { return _texture; }
    [[nodiscard]] const RHITexture &texture() const noexcept { return _texture; }
    [[nodiscard]] const ImageIO &image() const noexcept { return _image_io; }
    [[nodiscard]] ImageIO &image() noexcept { return _image_io; }
    [[nodiscard]] static ImageWrapper create(const TextureDesc &desc, Device *device);
    [[nodiscard]] TextureUploadCommand *upload() const noexcept;
    [[nodiscard]] TextureDownloadCommand *download() noexcept;
    void upload_immediately() const noexcept;
    void download_immediately() noexcept;
};

class ImagePool {
private:
    Device *_device{nullptr};
    map<uint64_t, ImageWrapper> _images;

public:
    explicit ImagePool(Device *device) : _device(device) {}
    [[nodiscard]] ImageWrapper &obtain_image(const TextureDesc &desc) noexcept;
    [[nodiscard]] bool is_contain(uint64_t hash) const noexcept { return _images.contains(hash); }
};

}