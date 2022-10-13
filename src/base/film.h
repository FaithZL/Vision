//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "rhi/common.h"
#include "node.h"

namespace vision {
using namespace ocarina;
class Film : public Node {
private:
    uint2 _resolution;

public:
    void prepare(RenderPipeline *pipeline) noexcept override;
    [[nodiscard]] uint _pixel_index(uint2 pixel) const { return pixel.y * _resolution.x + pixel.x; }
    [[nodiscard]] uint2 resolution() const noexcept { return _resolution; }
};
}// namespace vision