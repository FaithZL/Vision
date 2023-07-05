//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "dsl/common.h"
#include "rhi/common.h"
#include "core/hash.h"
#include "base/mgr/global.h"

namespace vision {
using namespace ocarina;

class DilateFilter : public Ctx {
private:
    uint _padding{};
    Shader<void(Texture, Texture)> _shader;

public:
    explicit DilateFilter(uint padding = 2)
        : _padding(padding) {}
    void set_padding(uint padding) noexcept { _padding = padding; }
    void compile() noexcept;
    [[nodiscard]] ShaderDispatchCommand *dispatch(const Texture &src,
                                                  const Texture &dst) const noexcept;
};

}// namespace vision