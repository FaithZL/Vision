//
// Created by Zero on 2024/2/17.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "scattering/interaction.h"

namespace vision {
using namespace ocarina;
struct PixelColor {
    array<float, 3> albedo{};
    array<float, 3> emission{};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision::PixelColor, albedo,emission) {};
// clang-format on

namespace vision {
using namespace ocarina;
struct PixelGeometry {
    array<float, 3> normal{};
    float normal_fwidth{};
    float depth_gradient{};
    float2 motion_vec{};
    float linear_depth{-1};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision::PixelGeometry, normal,
            normal_fwidth,depth_gradient, motion_vec,linear_depth) {};
// clang-format on
namespace vision {
using OCPixelGeometry = ocarina::Var<PixelGeometry>;
using OCPixelColor = ocarina::Var<PixelColor>;
}// namespace vision

namespace vision {

class FrameBuffer : public Node {
protected:
    /// save two frames of data
    RegistrableBuffer<PixelGeometry> GBuffer;

    /// save two frames of data , use for ReSTIR
    RegistrableBuffer<SurfaceData> _surface_buffer;

    RegistrableBuffer<PixelColor> _color_buffer;

public:
    using Desc = FrameBufferDesc;

public:
    explicit FrameBuffer(const FrameBufferDesc &desc);
    void prepare() noexcept override;
    virtual void compile() noexcept = 0;
};

}// namespace vision