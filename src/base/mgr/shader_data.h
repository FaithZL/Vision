//
// Created by Zero on 28/02/2023.
//

#pragma once

#include "rhi/common.h"

namespace vision {
using namespace ocarina;
class ShaderData : public ocarina::Managed<char> {
public:
    template<typename T>
    [[nodiscard]] uint add(T t) noexcept {
        uint ret = _size;

        return ret;
    }
};

}// namespace vision