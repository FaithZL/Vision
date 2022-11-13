//
// Created by Zero on 26/09/2022.
//

#include "image_pool.h"

namespace vision {

ImageWrapper ImageWrapper::create(const TextureDesc &desc, Device *device) {
    auto image_io = ImageIO::load(desc.fn, desc.color_space);
    auto image = device->create_image(image_io.resolution(), image_io.pixel_storage());
    return {move(image_io), move(image)};
}

ImageDownloadCommand *ImageWrapper::download() noexcept {
    return _image.download(_image_io.pixel_ptr());
}

ImageUploadCommand *ImageWrapper::upload() const noexcept {
    return _image.upload(_image_io.pixel_ptr());
}

ImageWrapper &ImagePool::obtain_image(const TextureDesc &desc) noexcept {
    uint64_t hash = desc.hash();
    if (!is_contain(hash)) {
        _images.insert(make_pair(hash, ImageWrapper::create(desc, _device)));
    }
    return _images[hash];
}
}// namespace vision