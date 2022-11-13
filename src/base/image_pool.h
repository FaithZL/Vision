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
    Image _image;

public:
    ImageWrapper() = default;
    [[nodiscard]] Image &image() noexcept { return _image; }
    [[nodiscard]] ImageUploadCommand upload() const noexcept;
    [[nodiscard]] ImageDownloadCommand download() noexcept;
};

class ImagePool {
private:
    map<uint64_t, ImageWrapper> _images;

public:
    ImagePool() = default;
    [[nodiscard]] ImageWrapper &obtain_image(const TextureDesc &desc) noexcept;
    [[nodiscard]] bool is_contain(uint64_t hash) const noexcept { return _images.contains(hash); }
};

}