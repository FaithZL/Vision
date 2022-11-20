//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "interaction.h"

namespace vision {

class Texture : public Node {
public:
    using Desc = TextureDesc;

public:
    template<typename T = float4>
    [[nodiscard]] static Float4 eval(const Texture *tex, const TextureEvalContext &ctx,
                                     T val = T{}) noexcept {
        float4 default_val = make_float4(0);
        if constexpr (is_scalar_v<T>) {
            default_val = make_float4(val);
        } else if constexpr (is_vector2_v<T>) {
            default_val = make_float4(val, 0, 0);
        } else if constexpr (is_vector3_v<T>) {
            default_val = make_float4(val, 0);
        } else {
            default_val = val;
        }
        return tex ? tex->eval(ctx) : Float4(val);
    }
    explicit Texture(const TextureDesc &desc) : Node(desc) {}
    [[nodiscard]] virtual Float4 eval(const TextureEvalContext &tev) const noexcept = 0;
    [[nodiscard]] virtual Float4 eval(const Float2 &uv) const noexcept = 0;
};

}// namespace vision