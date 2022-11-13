//
// Created by Zero on 26/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "descriptions/node_desc.h"

namespace vision {
using namespace ocarina;
class ImagePool {
private:
    map<uint64_t, Image> _images;

public:
    ImagePool() = default;

};

}