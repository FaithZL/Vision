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
    explicit Texture(const TextureDesc &desc) : Node(desc) {}
    [[nodiscard]] virtual Float4 eval(const TextureEvalContext &tev) const noexcept = 0;
    [[nodiscard]] virtual Float4 eval(const Float2 &uv) const noexcept = 0;
};
}// namespace vision