//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "dsl/common.h"
#include "rhi/common.h"
#include "core/hash.h"

namespace vision {
using namespace ocarina;

class DilateFilter : public Hashable {
private:
    uint _padding{};
    Managed<float4> _conv_kernel;
    Shader<void(Texture, Texture, Buffer<float>)> _shader;

public:
    explicit DilateFilter(uint padding = 2)
        : _padding(padding) {}
    void set_padding(uint padding) noexcept { _padding = padding; }
    void compile() noexcept;
};

}// namespace vision