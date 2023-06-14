//
// Created by Zero on 26/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "descriptions/node_desc.h"
#include "util/image_io.h"

namespace vision {
using namespace ocarina;

class Pipeline;

class ImageWrapper {
private:
    ImageIO _image_io;
    Texture _texture;
    uint _id{};

public:
    ImageWrapper() = default;
    ImageWrapper(ImageIO image_io, Texture image, uint id)
        : _image_io(ocarina::move(image_io)), _texture(ocarina::move(image)), _id(id) {}
    [[nodiscard]] Texture &texture() noexcept { return _texture; }
    [[nodiscard]] const Texture &texture() const noexcept { return _texture; }
    [[nodiscard]] const ImageIO &image() const noexcept { return _image_io; }
    [[nodiscard]] ImageIO &image() noexcept { return _image_io; }
    [[nodiscard]] uint id() const noexcept { return _id; }
    [[nodiscard]] static ImageWrapper create(const ShaderNodeDesc &desc, Pipeline *rp);
    [[nodiscard]] TextureUploadCommand *upload() const noexcept;
    [[nodiscard]] TextureDownloadCommand *download() noexcept;
    void upload_immediately() const noexcept;
    void download_immediately() noexcept;
};

class ImagePool {
private:
    map<uint64_t, ImageWrapper> _images;

public:
    ImagePool() = default;
    [[nodiscard]] ImageWrapper &obtain_image(const ShaderNodeDesc &desc) noexcept;
    void prepare() noexcept;
    [[nodiscard]] Pipeline *pipeline();
    [[nodiscard]] bool is_contain(uint64_t hash) const noexcept { return _images.contains(hash); }
};

}// namespace vision