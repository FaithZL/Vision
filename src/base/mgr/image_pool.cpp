//
// Created by Zero on 26/09/2022.
//

#include "image_pool.h"
#include "pipeline.h"
#include "global.h"
#include "rhi/device.h"

namespace vision {
using namespace ocarina;
ImageWrapper ImageWrapper::create(const ShaderNodeDesc &desc, Pipeline *rp) {
    ImageIO image_io;
    if (desc.sub_type == "constant") {
        image_io = ImageIO::pure_color(desc["value"].as_float4(), ocarina::LINEAR, make_uint2(1));
    } else {
        string color_space = desc["color_space"].as_string();
        if (color_space.empty()) {
            string fn = desc.file_name();
            color_space = (fn.ends_with(".exr") || fn.ends_with(".hdr")) ? "linear" : "srgb";
        }
        ColorSpace cs = color_space == "linear" ? LINEAR : SRGB;
        fs::path fpath = desc.file_name();
        if (!fpath.is_absolute()) {
            fpath = Global::instance().scene_path() / fpath;
        }
        image_io = ImageIO::load(fpath, cs);
    }
    auto texture = rp->device().create_texture(image_io.resolution(), image_io.pixel_storage());
    uint id = rp->register_texture(texture);
    return {ocarina::move(image_io), ocarina::move(texture), id};
}

ImageWrapper ImageWrapper::create(const fs::path &fn, ocarina::ColorSpace &cs, ocarina::float3 scale, bool need_device) {
    ImageIO image_io = ImageIO::load(fn, cs, scale);
    if (need_device) {
        auto texture = Global::instance().pipeline()->device().create_texture(image_io.resolution(), image_io.pixel_storage());
        uint id = Global::instance().pipeline()->register_texture(texture);
        return {ocarina::move(image_io), ocarina::move(texture), id};
    }
    return {ocarina::move(image_io)};
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

ImageWrapper &ImagePool::obtain_image(const fs::path &fn, ocarina::ColorSpace cs, ocarina::float3 scale, bool need_device) noexcept {
    uint64_t hash = hash64(fn.string(), cs, scale);
    if (!is_contain(hash)) {
        _images.insert(make_pair(hash, ImageWrapper::create(fn, cs, scale, need_device)));
    } else {
        OC_INFO_FORMAT("image load: find {} from image pool", fn.string().c_str());
    }
    return _images[hash];
}

ImageWrapper &ImagePool::obtain_image(const ShaderNodeDesc &desc) noexcept {
    uint64_t hash = desc.hash();
    if (!is_contain(hash)) {
        _images.insert(make_pair(hash, ImageWrapper::create(desc, pipeline())));
    } else {
        auto scene_path = Global::instance().scene_path();
        OC_INFO_FORMAT("image load: find {} from image pool", (scene_path / desc.file_name()).string().c_str());
    }
    return _images[hash];
}

void ImagePool::prepare() noexcept {
    for (auto &iter : _images) {
        pipeline()->stream() << iter.second.upload();
    }
    pipeline()->stream() << synchronize() << commit();
}

Pipeline *ImagePool::pipeline() {
    return Global::instance().pipeline();
}

ImagePool *ImagePool::s_image_pool = nullptr;

ImagePool &ImagePool::instance() {
    if (s_image_pool == nullptr) {
        s_image_pool = new ImagePool();
    }
    return *s_image_pool;
}

void ImagePool::destroy_instance() {
    if (s_image_pool) {
        delete s_image_pool;
        s_image_pool = nullptr;
    }
}

}// namespace vision