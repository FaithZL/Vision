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

namespace detail {
[[nodiscard]] inline Uint encode(Uint a, Uint b) noexcept {
    a = a << 16;
    return a | b;
}

[[nodiscard]] inline Uint2 decode(Uint arg) noexcept {
    Uint a = (0xffff0000 & arg) >> 16;
    Uint b = 0x0000ffff & arg;
    return make_uint2(a, b);
}
}// namespace detail

class DilateFilter : public Ctx {
private:
    int _padding{};
    using signature = void(Buffer<float4>, Buffer<float4>, Buffer<float4>, Buffer<float4>);
    Shader<signature> _shader;

public:
    explicit DilateFilter(int padding = 2);
    void set_padding(int padding) noexcept { _padding = padding; }
    void compile() noexcept;
    template<typename... Args>
    [[nodiscard]] ShaderInvoke operator()(Args &&...args) const noexcept {
        return _shader(OC_FORWARD(args)...);
    }
};

}// namespace vision