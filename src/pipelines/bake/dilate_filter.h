//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "dsl/dsl.h"
#include "rhi/common.h"
#include "core/hash.h"
#include "base/mgr/global.h"

namespace vision {
using namespace ocarina;

class DilateFilter : public Context {
private:
    int padding_{};
    using signature = void(Buffer<uint4>, Buffer<float4>, Buffer<float4>);
    Shader<signature> _shader;

public:
    explicit DilateFilter(int padding = 2);
    void set_padding(int padding) noexcept { padding_ = padding; }
    void compile() noexcept;
    template<typename... Args>
    [[nodiscard]] ShaderInvoke operator()(Args &&...args) const noexcept {
        return _shader(OC_FORWARD(args)...);
    }
};

}// namespace vision