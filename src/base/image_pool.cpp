//
// Created by Zero on 26/09/2022.
//

#include "image_pool.h"

namespace vision {

ImageWrapper &ImagePool::obtain_image(const TextureDesc &desc) noexcept {
    uint64_t hash = desc.hash();
//    if (is_contain(hash)) {
        return _images[hash];
//    }

}
}