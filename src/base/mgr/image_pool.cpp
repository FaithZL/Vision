//
// Created by Zero on 26/09/2022.
//

#include "image_pool.h"
#include "pipeline.h"
#include "global.h"
#include "rhi/device.h"

namespace vision {
using namespace ocarina;

RegistrableTexture ImagePool::load_texture(const ShaderNodeDesc &desc) noexcept {
    Image image_io;
    if (desc.sub_type == "constant") {
        image_io = Image::pure_color(desc["value"].as_float4(), ocarina::LINEAR, make_uint2(1));
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
        image_io = Image::load(fpath, cs);
    }
    RegistrableTexture ret{Global::instance().pipeline()->bindless_array()};
    ret.host_tex() = ocarina::move(image_io);
    ret.allocate_on_device(Global::instance().device(),desc.file_name());
    ret.register_self();
    return ret;
}

RegistrableTexture &ImagePool::obtain_texture(const ShaderNodeDesc &desc) noexcept {
    uint64_t hash = desc.hash();
    if (!is_contain(hash)) {
        textures_.insert(make_pair(hash, load_texture(desc)));
    } else {
        auto scene_path = Global::instance().scene_path();
        OC_INFO_FORMAT("image load: find {} from image pool", (scene_path / desc.file_name()).string().c_str());
    }
    return textures_[hash];
}

void ImagePool::prepare() noexcept {
    for (auto &iter : textures_) {
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