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
    uint _id{InvalidUI32};

public:
    ImageWrapper() = default;
    ImageWrapper(ImageIO image_io)
        : _image_io(ocarina::move(image_io)) {}
    ImageWrapper(ImageIO image_io, Texture image, uint id)
        : _image_io(ocarina::move(image_io)), _texture(ocarina::move(image)), _id(id) {}
    [[nodiscard]] Texture &texture() noexcept { return _texture; }
    [[nodiscard]] const Texture &texture() const noexcept { return _texture; }
    [[nodiscard]] uint channel_num() const noexcept { return _texture.channel_num(); }
    [[nodiscard]] const ImageIO &image() const noexcept { return _image_io; }
    [[nodiscard]] ImageIO &image() noexcept { return _image_io; }
    [[nodiscard]] uint id() const noexcept { return _id; }
    [[nodiscard]] static ImageWrapper create(const ShaderNodeDesc &desc, Pipeline *rp);
    [[nodiscard]] static ImageWrapper create(const fs::path &fn, ColorSpace &cs, float3 scale, bool need_device = false);
    [[nodiscard]] TextureUploadCommand *upload() const noexcept;
    [[nodiscard]] TextureDownloadCommand *download() noexcept;
    void upload_immediately() const noexcept;
    void download_immediately() noexcept;
};

class ImagePool {
private:
    map<uint64_t, ImageWrapper> _images;
    ImagePool() = default;
    static ImagePool *s_image_pool;
    ImagePool(const ImagePool &) = delete;
    ImagePool(ImagePool &&) = delete;
    ImagePool operator=(const ImagePool &) = delete;
    ImagePool operator=(ImagePool &&) = delete;
    [[nodiscard]] Pipeline *pipeline();

public:
    static ImagePool &instance();
    static void destroy_instance();
    [[nodiscard]] ImageWrapper &obtain_image(const ShaderNodeDesc &desc) noexcept;
    [[nodiscard]] ImageWrapper &obtain_image(const fs::path &fn, ColorSpace cs,
                                             float3 scale = make_float3(1.f), bool need_device = false) noexcept;
    void prepare() noexcept;
    [[nodiscard]] bool is_contain(uint64_t hash) const noexcept { return _images.contains(hash); }
};

}// namespace vision