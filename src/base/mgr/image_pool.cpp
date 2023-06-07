//
// Created by Zero on 26/09/2022.
//

#include "image_pool.h"
#include "render_pipeline.h"
#include "rhi/device.h"

namespace vision {
using namespace ocarina;
ImageWrapper ImageWrapper::create(const ShaderNodeDesc &desc, RenderPipeline *rp) {
    ImageIO image_io;
    if (desc.sub_type == "constant") {
        image_io = ImageIO::pure_color(desc["value"].as_float4(), ocarina::LINEAR, make_uint2(1));
    } else {
        ColorSpace cs = desc["color_space"].as_string() == "linear" ? LINEAR : SRGB;
        image_io = ImageIO::load(desc.file_name(), cs);
    }
    auto texture = rp->device().create_texture(image_io.resolution(), image_io.pixel_storage());
    uint id = rp->register_texture(texture);
    return {ocarina::move(image_io), ocarina::move(texture), id};
}

TextureDownloadCommand *ImageWrapper::download() noexcept {
    return _texture.download(_image_io.pixel_ptr());
}

TextureUploadCommand *ImageWrapper::upload() const noexcept {
    return _texture.upload(_image_io.pixel_ptr());
}

void ImageWrapper::upload_immediately() const noexcept {
    _texture.upload_immediately(_image_io.pixel_ptr());
}

void ImageWrapper::download_immediately() noexcept {
    _texture.download_immediately(_image_io.pixel_ptr());
}

ImageWrapper &ImagePool::obtain_image(const ShaderNodeDesc &desc) noexcept {
    uint64_t hash = desc.hash();
    if (!is_contain(hash)) {
        _images.insert(make_pair(hash, ImageWrapper::create(desc, _rp)));
    }
    return _images[hash];
}

void ImagePool::prepare() noexcept {
    for (auto &iter : _images) {
        _rp->stream() << iter.second.upload();
    }
    _rp->stream() << synchronize() << commit();
}

}// namespace vision