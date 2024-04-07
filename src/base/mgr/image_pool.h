//
// Created by Zero on 26/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "descriptions/node_desc.h"
#include "util/image.h"

namespace vision {
using namespace ocarina;
class Pipeline;
class ImagePool {
private:
    map<uint64_t, RegistrableTexture> _textures;
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
    [[nodiscard]] RegistrableTexture load_texture(const ShaderNodeDesc &desc) noexcept;
    [[nodiscard]] RegistrableTexture &obtain_texture(const ShaderNodeDesc &desc) noexcept;
    void prepare() noexcept;
    [[nodiscard]] bool is_contain(uint64_t hash) const noexcept { return _textures.contains(hash); }
};

}// namespace vision